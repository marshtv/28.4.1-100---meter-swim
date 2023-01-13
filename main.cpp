#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>

class Swimmer {
private:
	std::string name = "unknown";
	double speed = 1.0;
public:
	Swimmer(std::string in_name, double in_speed) {
		this->name = in_name;
		assert(in_speed >= 0);
		this->speed = in_speed;
	}

	std::string getName() {
		return this->name;
	}

	double getSpeed() {
		return this->speed;
	}
};

class SwimTrack {
private:
	int count = 0;
	double length = 100;
	Swimmer* swimmer = nullptr;
public:
	SwimTrack(int in_count, double in_length, std::string in_name, double in_speed) {
		assert(in_count >= 0);
		this->count = in_count;
		assert(in_length >= 0);
		this->length = in_length;
		this->swimmer = new Swimmer(in_name, in_speed);
	}

	SwimTrack(const SwimTrack &other) {
		this->count = other.count;
		this->length = other.length;
		this->swimmer = new Swimmer(other.swimmer->getName(), other.swimmer->getSpeed());
	}

	~SwimTrack() {
		delete this->swimmer;
	}

	int getCount() {
		return this->count;
	}

	double getLength() {
		return this->length;
	}

	Swimmer* getSwimmer() {
		return this->swimmer;
	}

	int getFinishSeconds() {
		return (int)std::round(this->length / this->getSwimmer()->getSpeed());
	}
};

class Pool {
private:
	int tracksNum = 6;
	double length = 100;
	SwimTrack** tracks = nullptr;
public:
	Pool(int in_tracksNum, double in_length) {
		assert(in_tracksNum >= 0);
		this->tracksNum = in_tracksNum;
		assert(in_length >= 0);
		this->length = in_length;
		if (in_tracksNum > 0 && in_length > 0) {
			tracks = new SwimTrack*[in_tracksNum];
			for (int i = 0; i < in_tracksNum; ++i) {
				std::cout << "Track #" << i << std::endl;
				std::cout << "Input swimmer's name and his speed (metre/sec):";
				std::string in_name;
				double in_speed;
				std::cin >> in_name >> in_speed;
				SwimTrack* swimTrack = new SwimTrack(i, this->length, in_name, in_speed);
				this->tracks[i] = swimTrack;
			}
		}
	}

	Pool(const Pool &other) {
		this->tracksNum = other.tracksNum;
		this->length = other.length;
		assert(other.tracks != nullptr);
		this->tracks = new SwimTrack*[this->tracksNum];
		for (int i = 0; i < this->tracksNum; ++i) {
			this->tracks[i] = other.tracks[i];
		}
	}

	~Pool() {
		delete[] this->tracks;
	}

	int getTracksNum() {
		return this->tracksNum;
	}

	double getLength() {
		return this->length;
	}

	SwimTrack* getTrackByCount(int in_count) {
		if(this->tracks == nullptr) return nullptr;
		else return this->tracks[in_count];
	}
};

struct tablo {
	int trackNum = 0;
	std::string name = "unknown";
	double swimmedLength = 0;
	bool bIsFinished = false;
};

std::vector<tablo> swimScoreboard;
std::mutex swimScoreboard_mutex;
std::vector<tablo> tracksWinList;
std::mutex tracksWinList_mutex;

void startSwim(SwimTrack* track) {
	swimScoreboard_mutex.lock();
	swimScoreboard[track->getCount()].trackNum = track->getCount();
	swimScoreboard[track->getCount()].name = track->getSwimmer()->getName();
	swimScoreboard[track->getCount()].bIsFinished = false;
	swimScoreboard[track->getCount()].swimmedLength = 0;
	while (!swimScoreboard[track->getCount()].bIsFinished) {
		swimScoreboard_mutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds (100));

		swimScoreboard_mutex.lock();
		swimScoreboard[track->getCount()].swimmedLength += (track->getSwimmer()->getSpeed()) / 10;

		if ((track->getLength() - swimScoreboard[track->getCount()].swimmedLength)
						< 0) { //std::numeric_limits<double>::epsilon()
			swimScoreboard[track->getCount()].bIsFinished = true;

			tracksWinList_mutex.lock();
			tracksWinList.push_back(swimScoreboard[track->getCount()]);
			tracksWinList_mutex.unlock();
		}
	}
	swimScoreboard_mutex.unlock();
}

void onlineTablo() {

}

int main() {
	std::cout << "Input swimming track's length:";
	double in_length;
	std::cin >> in_length;
	auto* pool = new Pool(6, in_length);

	std::cout << "-------------------------------------------------" << std::endl;
	std::cout << "Swimmer's List:" << std::endl;
	std::cout << "-------------------------------------------------" << std::endl;

	for (int i = 0; i < pool->getTracksNum(); ++i) {
		std::cout << "Track " << pool->getTrackByCount(i)->getCount() << " "
					<< pool->getTrackByCount(i)->getSwimmer()->getName() << " "
					<< "speed: " << pool->getTrackByCount(i)->getSwimmer()->getSpeed() << std::endl;
	}
	std::cout << "-------------------------------------------------" << std::endl;

	for (int i = 0; i < pool->getTracksNum(); ++i) {
		tablo in_tablo;
		swimScoreboard.push_back(in_tablo);
	}

	std::cout << "\n\t\tAttention...." << std::endl;
	for (int i = 3; i >= 0; --i) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (i > 0)
			std::cout << "\t\t" << i << " ...." << std::endl;
		else {
			std::cout << "\t\tSTART" << std::endl;
			break;
		}
	}
	std::cout << "-------------------------------------------------" << std::endl;

	std::thread start0(startSwim, pool->getTrackByCount(0));

	std::thread start1(startSwim, pool->getTrackByCount(1));

	std::thread start2(startSwim, pool->getTrackByCount(2));

	std::thread start3(startSwim, pool->getTrackByCount(3));

	std::thread start4(startSwim, pool->getTrackByCount(4));

	std::thread start5(startSwim, pool->getTrackByCount(5));

	bool bIsAll_Finished = false;
	while (!bIsAll_Finished) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		for (int j = 0; j < swimScoreboard.size(); ++j) {
			std::cout << swimScoreboard[j].name << "\t";
		}
		std::cout << std::endl;
		for (int j = 0; j < swimScoreboard.size(); ++j) {
			std::cout << swimScoreboard[j].swimmedLength << "\t";
		}
		std::cout << std::endl;

		for (int j = 0; j < swimScoreboard.size(); ++j) {
			if (!swimScoreboard[j].bIsFinished) break;
			bIsAll_Finished = true;
		}
		std::cout << "-------------------------------------------------" << std::endl;
	}

	start0.join();
	start1.join();
	start2.join();
	start3.join();
	start4.join();
	start5.join();

	std::cout << "Winner's Scoreboard:" << std::endl;
	std::cout << "-------------------------------------------------" << std::endl;
	//std::cout << "swimmers finished = " << tracksWinList.size() << std::endl;
	//std::cout << "-------------------------------------------------" << std::endl;
	for (int i = 0; i < tracksWinList.size(); ++i) {
		std::cout 	<< i + 1 << ". "
					<< pool->getTrackByCount(tracksWinList[i].trackNum)->getSwimmer()->getName()
					<< ",\tTime\t: "
					<< pool->getTrackByCount(tracksWinList[i].trackNum)->getFinishSeconds()
					<< "sec." << std::endl;
	}
	std::cout << "-------------------------------------------------" << std::endl;
	std::cout << "Exit program. Good by." << std::endl;

	return 0;
}
