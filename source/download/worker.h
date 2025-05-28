class speed {
public:
    speed(): finito(false), mod(0), prev(0), received(0), total(0) {}
    void start();
    void stop();
    void add(unsigned int b);
    void set_total(long long t);
    void set_download(download& d) { this->d = d; }

private:
    void draw(double progress, unsigned int speed);
    void human_readable(unsigned int b);

    thread t;
    atomic<bool> finito;
    atomic<unsigned int> bytes;
    atomic<unsigned int> received;
    long long total;
    int mod;
    double prev;
    download& d;
    static const int delay = 1000;
    static const double e_factor = 0.1;
}; 