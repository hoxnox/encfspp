/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160810 15:03:46 */

#include <string>
#include <memory>

#pragma once

namespace libencfs{

class EncfsMounterImpl;

/**@brief encfs encrypted directories mounter*/
class EncfsMounter
{
public:
	/**@name mount
	 * @{*/
	static std::unique_ptr<EncfsMounter>&&
	Mount(std::string encrypted_dir_path, std::string mount_dir_path,
	      std::string password, std::string config_file_path = "");
	EncfsMounter(std::string encrypted_dir_path, std::string mount_dir_path,
	             std::string password, std::string config_file_path = "");
	/**@}*/

	/**@name umount
	 * @{*/
	~EncfsMounter();
	/**@}*/

	/**@name utils
	 * @{*/
	bool IsMounted() const;
	std::string MountDirPath() const;
	std::string StrError() const;
	/**@}*/

	/**@name service
	 * @{*/
	//EncfsMounter(EncfsMounter&& move);
	//EncfsMouner& operator=(EncfsMounter&& move);
	/**@}*/

private:
	EncfsMounter() = delete;
	EncfsMounter(const EncfsMounter& copy) = delete;
	EncfsMounter& operator=(const EncfsMounter& move) = delete;
	EncfsMounterImpl* impl_;
};

} // namespace

