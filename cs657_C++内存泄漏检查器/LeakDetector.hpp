/********************
 * LeakDetector.hpp *
 ********************/

#ifndef _LEAK_DETECTOR_HPP_
#define _LEAK_DETECTOR_HPP_

void* operator new(size_t _size, char *_file, unsigned int _line);
void* operator new[](size_t size, char *_file, unsigned int _line);

#ifndef __NEW_OVERLOAD_IMPLEMENTATION__
#define new new(__FILE__, __LINE__)
#endif

class CLeakDetector {
public:
    static unsigned int m_uCallCount;
    CLeakDetector() noexcept {
        ++m_uCallCount;
    }
    ~CLeakDetector() noexcept {
        if (--m_uCallCount == 0)
            LeakDetector();
    }
private:
    static unsigned int LeakDetector() noexcept;
};

static CLeakDetector exitCounter;


#endif
