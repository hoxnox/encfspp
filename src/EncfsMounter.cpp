/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160810 15:03:46 */

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <thread>
#include <logging.hpp>
#include <memory>
#include <future>

#define FUSE_USE_VERSION 26
#include <fuse.h>
extern "C" void fuse_unmount_compat22(const char *mountpoint);
#define fuse_unmount fuse_unmount_compat22

#include <encfs/Context.h>
#include <encfs/Error.h>
#include <encfs/FileUtils.h>
#include <encfs/MemoryPool.h>
#include <encfs/encfs.h>

#include <openssl/crypto.h>

#include <libencfs/EncfsMounter.hpp>

namespace libencfs {

using namespace encfs;

static void*
encfs_init(fuse_conn_info *conn)
{
	EncFS_Context *ctx = (EncFS_Context *)fuse_get_context()->private_data;
	conn->async_read = true;
	return (void *)ctx;
}

class EncfsMounterImpl
{
private:
	EncfsMounterImpl()
	{
		OPENSSL_init();
		memset(&encfs_oper, 0, sizeof(fuse_operations));
		encfs_oper.getattr = encfs_getattr;
		encfs_oper.readlink = encfs_readlink;
		encfs_oper.readdir = encfs_readdir;
		encfs_oper.mknod = encfs_mknod;
		encfs_oper.mkdir = encfs_mkdir;
		encfs_oper.unlink = encfs_unlink;
		encfs_oper.rmdir = encfs_rmdir;
		encfs_oper.symlink = encfs_symlink;
		encfs_oper.rename = encfs_rename;
		encfs_oper.link = encfs_link;
		encfs_oper.chmod = encfs_chmod;
		encfs_oper.chown = encfs_chown;
		encfs_oper.truncate = encfs_truncate;
		encfs_oper.utime = encfs_utime;  // deprecated for utimens
		encfs_oper.open = encfs_open;
		encfs_oper.read = encfs_read;
		encfs_oper.write = encfs_write;
		encfs_oper.statfs = encfs_statfs;
		encfs_oper.flush = encfs_flush;
		encfs_oper.release = encfs_release;
		encfs_oper.fsync = encfs_fsync;
		encfs_oper.init = encfs_init;
		//encfs_oper.destroy = encfs_destroy;
		encfs_oper.create = encfs_create;
		encfs_oper.ftruncate = encfs_ftruncate;
		encfs_oper.fgetattr = encfs_fgetattr;
		encfs_oper.utimens = encfs_utimens;

		opts->ownerCreate = true;
		opts->idleTracking = true;
		opts->createIfNotFound = false;
		//opts->useStdin = true;
	}
	~EncfsMounterImpl() {}
	std::string mountpoint;
	std::unique_ptr<std::thread> thread;
	fuse_operations encfs_oper;
	std::shared_ptr<EncFS_Opts> opts{new EncFS_Opts};
	std::shared_ptr<EncFS_Context> ctx{new EncFS_Context};
	RootPtr rootInfo;
	struct fuse *fuse{nullptr};
friend class EncfsMounter;
};

std::function<std::string(std::string promt)> EncfsMounter::readpassphrase_ = {};

std::unique_ptr<EncfsMounter>&&
EncfsMounter::Mount(std::string enc_dir_path,
                    std::string mnt_dir_path,
                    std::string cfg_file_path /*= ""*/)
{
	return std::move(std::unique_ptr<EncfsMounter>(
		new EncfsMounter(enc_dir_path, mnt_dir_path, cfg_file_path)));
}


inline std::string
slashTerminate(const std::string &src)
{
	std::string result = src;
	if (result[result.length() - 1] != '/') result.append("/");
	return result;
}

EncfsMounter::EncfsMounter(std::string encrypted_dir_path, std::string mount_dir_path,
             std::string config_file_path /*= ""*/)
	: impl_(new EncfsMounterImpl())
{
	VLOG << "Attempt to mount \"" << encrypted_dir_path
	     << "\" to \"" << mount_dir_path << "\""
	     << " with config \"" << config_file_path << "\"";

	impl_->mountpoint = mount_dir_path;
	impl_->opts->rootDir = slashTerminate(encrypted_dir_path);
	impl_->opts->mountPoint = slashTerminate(mount_dir_path);

	impl_->ctx->publicFilesystem = impl_->opts->ownerCreate;
	if (setenv("ENCFS6_CONFIG", config_file_path.c_str(), 1) != 0)
	{
		LOG(ERROR) << "Error setting configuration file path.";
		return;
	}
	impl_->rootInfo = initFS(impl_->ctx.get(), impl_->opts);
	if (!impl_->rootInfo)
	{
		LOG(ERROR) << "Error initializing filesystem.";
		return;
	}
	impl_->ctx->setRoot(impl_->rootInfo->root);
	impl_->ctx->opts = impl_->opts;
	umask(0);

    std::promise<void> start_notifier;
	std::future<void> start_waiter = start_notifier.get_future();
	impl_->thread.reset(new std::thread([this](std::promise<void>&& start_notifier)
		{
			//char* mountpoint = new char[impl_->mountpoint.length()];
			// std::copy(impl_->mountpoint.begin(), impl_->mountpoint.end(), mountpoint);
			char name[] = {'l', 'i', 'b', 'e', 'n', 'c', 'f', 's', '\0'};
			char minus_f[] = {'-', 'f', '\0'};
			char* fuse_argv[4] = { name, minus_f, const_cast<char*>(impl_->mountpoint.data()), nullptr };
			int fuse_argc = 3;
			int multithreaded = 0;
			char* mountpoint  = nullptr;
			impl_->fuse = fuse_setup(fuse_argc, fuse_argv, &impl_->encfs_oper, sizeof(impl_->encfs_oper),
					&mountpoint, &multithreaded, (void*)impl_->ctx.get());
			if (impl_->fuse == nullptr)
				return;
			start_notifier.set_value();
			int res = fuse_loop(impl_->fuse);
			if (res != 0)
			{
				LOG(ERROR) << "Error in fuse_loop. "
				           << "Message: " << strerror(errno);
			}
			fuse_teardown(impl_->fuse, mountpoint);
			impl_->rootInfo.reset();
			impl_->ctx->setRoot(std::shared_ptr<DirNode>());
			MemoryPool::destroyAll();
		}, std::move(start_notifier)));
	if(start_waiter.valid())
        start_waiter.wait();
}

EncfsMounter::~EncfsMounter()
{
	fuse_unmount(impl_->mountpoint.c_str());
	if (impl_->thread->joinable())
		impl_->thread->join();
	delete(impl_);
}

bool
EncfsMounter::IsMounted() const
{
	// TODO: check if mounted
	return false;
}

} // namespace


