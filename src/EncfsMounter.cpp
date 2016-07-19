#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <thread>
#include "Context.h"
#include "Error.h"
#include "FileUtils.h"
#include "MemoryPool.h"
#include "config.h"
#include "encfs.h"
#include "fuse.h"
#include "openssl.h"

// Fuse version >= 26 requires another argument to fuse_unmount, which we
// don't have.  So use the backward compatible call instead..
extern "C" void fuse_unmount_compat22(const char *mountpoint);
#define fuse_unmount fuse_unmount_compat22

class EncfsMounter
{
public:
	static std::shared_ptr<EncfsMounter> GetInstance();
	static int Mount(std::string config_path, std::string password,
			std::string encrypted_dir_path, std::string mount_dir_path);
	static int Umount(std::string mounted_dir_path);
	static int IsMounted(std::string mounted_dir_path) const;
private:
	std::string slashTerminate(const string &src)
	std::shared_ptr<EncfsMounter> instance_;
};

inline std::string
EncfsMounter::slashTerminate(const std::string &src)
{
	std::string result = src;
	if (result[result.length() - 1] != '/') result.append("/");
	return result;
}

static void*
EncfsMounter::encfs_init(fuse_conn_info *conn)
{
	EncFS_Context *ctx = (EncFS_Context *)fuse_get_context()->private_data;
	conn->async_read = true;
	return (void *)ctx;
}

static void
EncfsMounter::encfs_destroy(void *_ctx) {}


int
main(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " <from> <to>" << std::endl;
    return 0;
  }

  el::Loggers::setVerboseLevel(1);
  encfs::initLogging(true);

  VLOG(1) << "Root directory: " << argv[1];

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
  encfs_oper.destroy = encfs_destroy;
  encfs_oper.create = encfs_create;
  encfs_oper.ftruncate = encfs_ftruncate;
  encfs_oper.fgetattr = encfs_fgetattr;
  encfs_oper.utimens = encfs_utimens;

  openssl_init(false);

  std::shared_ptr<EncFS_Opts> opts(new EncFS_Opts());
  opts->ownerCreate = true;
  opts->idleTracking = true;
  opts->createIfNotFound = false;
  opts->rootDir = slashTerminate(argv[1]);
  opts->mountPoint = slashTerminate(argv[2]);
  //opts->useStdin = true;
  auto ctx = std::shared_ptr<EncFS_Context>(new EncFS_Context);
  ctx->publicFilesystem = opts->ownerCreate;
  RootPtr rootInfo = initFS(ctx.get(), opts);

  std::thread* t;
  struct fuse *fuse;
  if (rootInfo)
  {
    ctx->setRoot(rootInfo->root);
    ctx->opts = opts;
    umask(0);
    t = new std::thread([&argv, &argc, &encfs_oper, &ctx, &fuse]() 
        {
          char* fuse_argv[3] = { argv[0], "-f", argv[2] };
          int fuse_argc = 3;
          int multithreaded = 1;
          fuse = fuse_setup(fuse_argc, fuse_argv, &encfs_oper, sizeof(encfs_oper), &argv[2],
                     &multithreaded, (void*)ctx.get());
          if (fuse == NULL)
            return 1;
          int res = fuse_loop(fuse);
          fuse_teardown(fuse, argv[2]);
        });
  }
  fuse_exit(fuse);
  fuse_unmount(opts->mountPoint.c_str());
  t->join();
  delete t;
  // cleanup so that we can check for leaked resources..
  rootInfo.reset();
  ctx->setRoot(std::shared_ptr<DirNode>());

  MemoryPool::destroyAll();
  openssl_shutdown(false);

  return 0;
}

