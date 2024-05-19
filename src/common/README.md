# PECAN!!!

If you want platform specific code in the header file, think a lot beforehand 
(pecan architecture is averse to platform-specific headers, we want a general implementation
for mbed and arduino like polymorphism). 

If you still think its necessary, do like so
```cpp
#ifdef __MBED__
#endif

#ifdef ARDUNO_AVR_UNO
#endif 
```

TODO:
- Fix matcher functions and make it platform-agnostic