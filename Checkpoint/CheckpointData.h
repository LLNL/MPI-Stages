//#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <string>
#include <vector>

/*
 User-defined checkpoint data.
 */
class CheckpointData {
private:
	int iterNum;
	std::vector<int> arr;

	friend class boost::serialization::access;

	template <typename Archive>
	void serialize(Archive& ar, const unsigned version) {
		ar& iterNum;
		ar& arr;
	}
public:
	//CheckpointData() {}
	CheckpointData(int n, std::vector<int> vec) {
		iterNum = n;
		arr = vec;
	}

	void setNum(int number) {
		iterNum = number;
	}
	int getNum() {
		return iterNum;
	}

	void setArr(std::vector<int> vec) {
		arr = vec;
	}

	std::vector<int> getArr() {
		return arr;
	}


};
