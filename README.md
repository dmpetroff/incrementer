## Introduction

I assume we're speaking about synchronization primitives, not atomics

## 1b. Issues with adding more threads

If we add more even/odd "workers" then `conditional_variable::notify_one()` will woke up
a random thread possibly creating a deadlock if even incrementer awokes another even incrementer.

Also it is worth mentioning that increasing number of threads won't speed things up just because
only one increment operation is allowed at time.

## 1c. How to fix

1b can be fixed in several possible ways but I would prefer to introduce separate condition
variable for each group and still use `notify_one()` to wake up worker from "complementing group".

Simply replacing `notify_one()` with `notify_all()` should also do the trick. But it would
introduce some wakeups that won't proceed to counter increment which is a waste of resources.
But in theory this can speed things up as it allows multiple increments to happen without waiting
on conditional variable between them. 
