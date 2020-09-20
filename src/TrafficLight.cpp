#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> uniq_lock(_mutex);
    
    while(_queue.empty()) {
        _condvar.wait(uniq_lock);
    }

    T val = std::move(_queue.front());
    _queue.pop_front();
    return val;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lockq(_mutex);
    _queue.clear();
    _queue.push_back(std::move(msg));

    _condvar.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        if (_msgQueue.receive() == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 

    threads.emplace_back(std::thread([this](){cycleThroughPhases();}));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rd;  
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(4.0, 6.0);

    auto time_to_wait = long(dis(gen) * 1000);
    auto previousPhaseTime = std::chrono::steady_clock::now();

    while(true) {

    
    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - previousPhaseTime);
    if ((time_diff.count() >= time_to_wait and time_diff.count() <= time_to_wait) || time_diff.count() > 6000) {

           
            time_to_wait = long(dis(gen) * 1000);

            auto currentPhase = getCurrentPhase();
             std::cout<<"TrafficLight Time diff (ms): "<< time_diff.count() << std::endl;

            if (currentPhase == TrafficLightPhase::red) {
                _msgQueue.send(TrafficLightPhase::green);
                _currentPhase = TrafficLightPhase::green;

             } else {
                _msgQueue.send(TrafficLightPhase::red);
                _currentPhase = TrafficLightPhase::red;
        }

         previousPhaseTime = std::chrono::steady_clock::now();


    } 


    std::this_thread::sleep_for(std::chrono::milliseconds(1));


    }




}

