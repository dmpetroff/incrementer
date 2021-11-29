#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <iostream>
#include <functional>

template<int Remainder, int Factor>
void
incrementer(int &counter, int till, std::mutex &lock,
			std::condition_variable &notify, std::condition_variable &wait,
			int &increment_counter)
{
	increment_counter = 0;
	for (;;) {
		std::unique_lock<std::mutex> lk(lock);
		while (counter < till && counter % Factor != Remainder)
			wait.wait(lk);
		if (counter >= till)
			break;
		int next = ++counter;
		lk.unlock();

		increment_counter++;
		if (next < till) {
			notify.notify_one();
		} else {
			// make everyone awake and realize we've done
			notify.notify_all();
			wait.notify_all();
			break;
		}
	}
}

// fsf = False Sharing Friendly - an integer counter and 60 bytes of padding
// to fill 64-byte cacheline
struct fsf_counter {
	int value = 0;
	int padding[15];
};


int main(int argc, char **argv)
{
	if (argc != 3 || std::string_view(argv[1]) == "--help") {
		std::cout << "Usage: " << argv[0] << " increment-till-this-value how-many-incrementers-of-each-type-to-use" << std::endl;
		return 1;
	}

	int count = 0, till = std::atoi(argv[1]), ninc = std::stoi(argv[2]);
	int N = ninc * 2;
	std::mutex lock;
	std::condition_variable group0, group1;
	std::vector<std::thread> workers;
	std::vector<fsf_counter> counters(N);

	workers.reserve(N);

	for (int i = 0; i < ninc; i++) {
		workers.emplace_back([&, i]() {
			incrementer<0, 2>(count, till, lock, group1, group0, counters[i * 2].value);
		});
		workers.emplace_back([&, i]() {
			incrementer<1, 2>(count, till, lock, group0, group1, counters[i * 2 + 1].value);
		});
	}

	for (auto &t : workers)
		t.join();

	std::cout << "New counter value is " << count << std::endl;
	std::cout << "Actual increments performed by thread" << std::endl;
	for (int i = 0; i < N; i++)
		std::cout << "  thread #" << i << " (group "
			<< (i % 2 == 0 ? "even" : "odd") << "): " 
			<< counters[i].value << std::endl;
	return 0;
}
