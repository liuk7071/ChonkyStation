#pragma once

#include <helpers.hpp>
#include <queue>
#include <set>


constexpr auto schedulerMaxEntries = 64;

// https://stackoverflow.com/questions/19467485/how-to-remove-element-not-at-top-from-priority-queue
template<typename T>
class pqueue : public std::priority_queue<T, std::vector<T>, std::greater<T>> {
public:
    std::vector<T>& get_container() {
        return this->c;
    }

    bool remove(const T& value) {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        
        if (it == this->c.end()) {
            return false;
        }

        if (it == this->c.begin()) {
            // Deque the top element
            this->pop();
        }
        else {
            // Remove element and re-heap
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
        }

        return true;
    }
};

class Scheduler {
public:
	u64 time = 0;
	void tick(u64 cycles);

	struct Event {
		void (*functionPtr)(void*) = nullptr;
		void* data = nullptr;
		u64 time = 0;
        std::string name = "Default";

        bool operator==(const Event& other) const {
            return (functionPtr == other.functionPtr) && (data == other.data) && (time == other.time) && (name == other.name);
        }

		bool operator>(const Event& other) const {
			return time > other.time;
		}
	};

	pqueue<Event> events;
	void push(void (*functionPtr)(void*), u64 time, void* data, std::string name = "Default");
	void deleteAllEventsOfName(std::string name);
};