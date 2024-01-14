#include <iostream>
#include <sys/file.h>

/**
 * @brief 这是一个文件锁，具有有RAII功能，构造函数有两个参数，第一个参数是文件路径，第二个参数是是否使用写锁，
 *        true - 写锁，false - 读锁，该锁可以提供一写多读的功能，可以有多个锁同时读取，但是同一时间只能有一
 *        个锁写，使用方式如下：
 * 
 *        DBusFileLock fileLock("example.txt", true); // 使用写锁模式
 * 
 *        DBusFileLock fileLock("example.txt"); // 使用读锁模式（默认为读锁，因此可以不写 false）
 */

namespace opendbus {

class DBusFileLock {
private:
    int iFd;

public:
    DBusFileLock(const std::string& strFilePath, bool bWritable = false)
    {
        // 打开文件
        iFd = open(strFilePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (iFd == -1) {
            throw std::runtime_error("could not open file");
        }


        // 请求锁的类型
        int iLockType = bWritable ? (LOCK_EX | LOCK_NB) : (LOCK_SH | LOCK_NB);
        if (flock(iFd, iLockType) == -1) {
            throw std::runtime_error("lock failed");
        }
    }

    ~DBusFileLock()
    {
        // 解锁
        unlock();
        // 关闭文件
        close(iFd);
    }

    void unlock() {

        if (flock(iFd, LOCK_UN) == -1) {
            throw std::runtime_error("unlock failed");
        }
    }
};

} // namespace hicore
