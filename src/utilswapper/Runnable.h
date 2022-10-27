#ifndef RUNNABLE_H_
#define RUNNABLE_H_

class Runnable {
public:
    Runnable();
    virtual ~Runnable();

    void runnable();

private:
    virtual void run() = 0;
};

#endif /* RUNNABLE_H_ */
