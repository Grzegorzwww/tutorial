// Zadanie 39 (C++): automat stanow (typowy wzorzec w embedded)
//
// Przejscia:
//   Idle    + Start  -> Running
//   Running + Pause  -> Paused
//   Paused  + Resume -> Running
//   Running + Stop   -> Stopped
//   Paused  + Stop   -> Stopped
// Wszystkie inne kombinacje (w tym kazde zdarzenie w stanie Stopped) NIE zmieniaja stanu.
//
//   next_state:   funkcja przejscia (najlepiej przez constexpr tablice [stan][zdarzenie])
//   to_string:    "Idle", "Running", "Paused", "Stopped"
//   run:          zaczyna w Idle i przetwarza zdarzenia po kolei
//   is_terminal:  true tylko dla Stopped
#include <array>
#include <cassert>
#include <iostream>
#include <span>
#include <string_view>

enum class State { Idle, Running, Paused, Stopped };
enum class Event { Start, Pause, Resume, Stop };

State next_state(State s, Event e) {
    // TODO
    return s;
}

std::string_view to_string(State s) {
    // TODO
    return "";
}

State run(std::span<const Event> events) {
    // TODO
    return State::Idle;
}

bool is_terminal(State s) {
    // TODO
    return false;
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
