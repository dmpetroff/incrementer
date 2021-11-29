#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <iostream>

template<int Remainder, int Factor>
void
incrementer(int &counter, int till, std::mutex &lock, std::condition_variable &notify)
{
	std::unique_lock<std::mutex> lk(lock, std::defer_lock);
	for (;;) {
		lock.lock();
		while (counter < till && counter % Factor != Remainder)
			notify.wait(lk);
		if (counter >= till)
			break;
		int next = ++counter;
		lock.unlock();
		notify.notify_one();
		if (next >= till)
			break;
	}
}


int main(int argc, char **argv)
{
	if (argc != 2 || std::string_view(argv[1]) == "--help") {
		std::cout << "Usage: " << argv[0] << " increment-till-this-value" << std::endl;
		return 1;
	}

	int count = 0, till = std::atoi(argv[1]);
	std::mutex lock;
	std::condition_variable notify;
	std::vector<std::thread> workers;

	workers.reserve(2);
	workers.emplace_back([&]() { incrementer<0, 2>(count, till, lock, notify); });
	workers.emplace_back([&]() { incrementer<1, 2>(count, till, lock, notify); });

	for (auto &t : workers)
		t.join();

	std::cout << "New counter value is " << count << std::endl;
	return 0;
}
