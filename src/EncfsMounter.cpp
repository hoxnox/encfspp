/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160810 15:03:46 */

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <thread>
#include <logging.hpp>

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

class EncfsMounterImpl
{
private:
	EncfsMounterImpl() {}
	~EncfsMounterImpl() {}
friend class EncfsMounter;
};

std::unique_ptr<EncfsMounter>&&
EncfsMounter::Mount(std::string encrypted_dir_path, std::string mount_dir_path,
                    std::string password, std::string config_file_path /*= ""*/)
{
	return std::move(std::unique_ptr<EncfsMounter>(new EncfsMounter(
		encrypted_dir_path, mount_dir_path, password, config_file_path)));
}


inline std::string
slashTerminate(const std::string &src)
{
	std::string result = src;
	if (result[result.length() - 1] != '/') result.append("/");
	return result;
}

static void*
encfs_init(fuse_conn_info *conn)
{
	EncFS_Context *ctx = (EncFS_Context *)fuse_get_context()->private_data;
	conn->async_read = true;
	return (void *)ctx;
}

EncfsMounter::EncfsMounter(std::string encrypted_dir_path, std::string mount_dir_path,
             std::string password, std::string config_file_path /*= ""*/)
	: impl_(new EncfsMounterImpl())
{
	fuse_operations encfs_oper;
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
	
	OPENSSL_init();
	
	std::shared_ptr<EncFS_Opts> opts(new EncFS_Opts());
	opts->ownerCreate = true;
	opts->idleTracking = true;
	opts->createIfNotFound = false;
	opts->rootDir = slashTerminate(encrypted_dir_path);
	opts->mountPoint = slashTerminate(mount_dir_path);
	//opts->useStdin = true;
	auto ctx = std::shared_ptr<EncFS_Context>(new EncFS_Context);
	ctx->publicFilesystem = opts->ownerCreate;
	RootPtr rootInfo = initFS(ctx.get(), opts);
	if (!rootInfo)
	{
		LOG(ERROR) << "Error initializing filesystem.";
		return;
	}
	
	std::thread* t;
	struct fuse *fuse;
	ctx->setRoot(rootInfo->root);
	ctx->opts = opts;
	umask(0);
	t = new std::thread([&mount_dir_path, &encfs_oper, &ctx, &fuse]() 
		{
			char minus_f[3] = "-f";
			std::unique_ptr<char> mount_dir_path_c(new char[mount_dir_path.length()]);
			memset(mount_dir_path_c.get(), 0, mount_dir_path.length());
			std::copy(mount_dir_path.begin(), mount_dir_path.end(), mount_dir_path_c.get());
			char* fuse_argv[3] = { nullptr, minus_f, mount_dir_path_c.get() };
			int fuse_argc = 3;
			int multithreaded = 1;
			char* mount_dir_path_c_ptr = mount_dir_path_c.get();
			fuse = fuse_setup(fuse_argc, fuse_argv, &encfs_oper, sizeof(encfs_oper),
					&mount_dir_path_c_ptr, &multithreaded, (void*)ctx.get());
			if (fuse == NULL)
				return;
			int res = fuse_loop(fuse);
			fuse_teardown(fuse, mount_dir_path_c.get());
		});
	std::this_thread::sleep_for(std::chrono::seconds(5));
	fuse_exit(fuse);
	fuse_unmount(opts->mountPoint.c_str());
	t->join();
	delete t;
	// cleanup so that we can check for leaked resources..
	rootInfo.reset();
	ctx->setRoot(std::shared_ptr<DirNode>());
	
	MemoryPool::destroyAll();
	//openssl_shutdown(false);
}

EncfsMounter::~EncfsMounter()
{
	// TODO: umount here
	delete(impl_);
}

bool
EncfsMounter::IsMounted() const
{
	// TODO: check if mounted
	return false;
}

std::string
EncfsMounter::MountDirPath() const
{
	// TODO:
	return "";
}

std::string
EncfsMounter::StrError() const
{
	// TODO:
	return "";
}

} // namespace


