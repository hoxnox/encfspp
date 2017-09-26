/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160810 15:03:46 */

#include <string>
#include <memory>
#include <functional>

#pragma once

namespace encfspp{

class EncfsMounterImpl;

/**@brief encfs encrypted directories mounter*/
class EncfsMounter
{
public:
	static void set_readpassphrase(std::function<std::string(std::string)> readpassphrase)
	{
		readpassphrase_ = readpassphrase;
	}
	static std::string readpassphrase(std::string prompt)
	{
		return readpassphrase_(prompt);
	}

	/**@name mount
	 * @{*/
	static std::unique_ptr<EncfsMounter>&&
	Mount(std::string encrypted_dir_path,
	      std::string mount_dir_path,
	      std::string config_file_path = "");

	EncfsMounter(std::string encrypted_dir_path,
	             std::string mount_dir_path,
	             std::string config_file_path = "");
	/**@}*/

	/**@name umount
	 * @{*/
	~EncfsMounter();
	/**@}*/

	/**@name utils
	 * @{*/
	bool IsMounted() const;
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
	static std::function<std::string(std::string promt)> readpassphrase_;
};

} // namespace

