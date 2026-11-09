#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>
#include <string_view>

enum class State { Idle, Running, Paused, Stopped };
enum class Event { Start, Pause, Resume, Stop };

namespace {
constexpr std::size_t kStates = 4;
constexpr std::size_t kEvents = 4;

constexpr std::array<std::array<State, kEvents>, kStates> make_table() {
    using enum State;
    using enum Event;
    // domyslnie: zdarzenie nie zmienia stanu
    std::array<std::array<State, kEvents>, kStates> t{};
    for (std::size_t s = 0; s < kStates; ++s) {
        for (std::size_t e = 0; e < kEvents; ++e) t[s][e] = static_cast<State>(s);
    }
    t[static_cast<std::size_t>(Idle)][static_cast<std::size_t>(Start)] = Running;
    t[static_cast<std::size_t>(Running)][static_cast<std::size_t>(Pause)] = Paused;
    t[static_cast<std::size_t>(Paused)][static_cast<std::size_t>(Resume)] = Running;
    t[static_cast<std::size_t>(Running)][static_cast<std::size_t>(Stop)] = Stopped;
    t[static_cast<std::size_t>(Paused)][static_cast<std::size_t>(Stop)] = Stopped;
    return t;
}

constexpr auto kTable = make_table();
}  // namespace

State next_state(State s, Event e) {
    return kTable[static_cast<std::size_t>(s)][static_cast<std::size_t>(e)];
}

std::string_view to_string(State s) {
    switch (s) {
        case State::Idle: return "Idle";
        case State::Running: return "Running";
        case State::Paused: return "Paused";
        case State::Stopped: return "Stopped";
    }
    return "?";
}

State run(std::span<const Event> events) {
    State s = State::Idle;
    for (Event e : events) s = next_state(s, e);
    return s;
}

bool is_terminal(State s) {
    return s == State::Stopped;
}

int main() {
    assert(next_state(State::Idle, Event::Start) == State::Running);
    assert(next_state(State::Idle, Event::Pause) == State::Idle);
    assert(next_state(State::Idle, Event::Stop) == State::Idle);
    assert(next_state(State::Running, Event::Pause) == State::Paused);
    assert(next_state(State::Running, Event::Start) == State::Running);
    assert(next_state(State::Paused, Event::Resume) == State::Running);
    assert(next_state(State::Running, Event::Stop) == State::Stopped);
    assert(next_state(State::Paused, Event::Stop) == State::Stopped);
    assert(next_state(State::Stopped, Event::Start) == State::Stopped);

    assert(to_string(State::Idle) == "Idle");
    assert(to_string(State::Running) == "Running");
    assert(to_string(State::Paused) == "Paused");
    assert(to_string(State::Stopped) == "Stopped");

    std::array<Event, 4> full{Event::Start, Event::Pause, Event::Resume, Event::Stop};
    assert(run(full) == State::Stopped);
    std::array<Event, 2> half{Event::Start, Event::Pause};
    assert(run(half) == State::Paused);
    std::array<Event, 3> noisy{Event::Pause, Event::Resume, Event::Start};
    assert(run(noisy) == State::Running);
    std::array<Event, 3> late{Event::Start, Event::Stop, Event::Start};
    assert(run(late) == State::Stopped);
    assert(run({}) == State::Idle);

    assert(is_terminal(State::Stopped));
    assert(!is_terminal(State::Idle) && !is_terminal(State::Running) && !is_terminal(State::Paused));

    std::cout << "OK\n";
}
