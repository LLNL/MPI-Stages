#include <string>

class Config {
private:
	std::string dirPath;
	std::string localDir;
	std::string fileName;
public:
	Config();
	Config(std::string configFile);
	void setDirPath(std::string path);
	void setLocalDir(std::string dir);
	std::string getDirPath();
	std::string getLocalDir();
	void getConfiguration();
};
