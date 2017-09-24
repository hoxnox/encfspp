/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160719 12:09:57 */

#pragma once

/**@param config_path Configuration file destination. If empty - look into
 * `encrypted_dir_path/.encfs6.xml`*/
int libencfs_mount(const char* encrypted_dir_path, const char* mount_dir_path, 
                   const char* password, const char* config_path = NULL);

int libencfs_umount(const char* mount_dir_path);

int libencfs_is_mounted(const char* mount_dir_path);

