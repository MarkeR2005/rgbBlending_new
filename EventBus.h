//---------------------------------------------------------------------------

#include <unordered_map>
#ifndef EventBusH
#define EventBusH
//---------------------------------------------------------------------------
class EventBus {
public:
	using Callback = std::function<void()>;

	static EventBus& instance() {
        static EventBus bus;
		return bus;
    }

	int subscribe(Callback cb) {
        int id = ++counter_;
		listeners_[id] = std::move(cb);
        return id;
	}

    void unsubscribe(int id) {
		listeners_.erase(id);
	}

	void emit() {
		for (auto& [id, cb] : listeners_)
            cb();
    }

private:
	std::unordered_map<int, Callback> listeners_;
	int counter_ = 0;
};

#endif
