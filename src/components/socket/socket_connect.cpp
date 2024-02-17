#include "socket_connect.h"
#include "socket_channel.h"
#include "event_loop.h"

#include <functional>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

using namespace std::placeholders;

namespace speed::mq {

socket_connect::socket_connect(event_loop* loop, const std::string& name, int sockfd)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      reading_(true),
      // socket_(new Socket(sockfd)),
      channel_(new socket_channel(*loop, sockfd)),
      high_water_mark_(64 * 1024 * 1024) {
    channel_->set_read_callback(std::bind(&socket_connect::handle_read, this));
    channel_->set_write_callback(std::bind(&socket_connect::handle_write, this));
    channel_->set_close_callback(std::bind(&socket_connect::handle_close, this));
    channel_->set_error_callback(std::bind(&socket_connect::handle_error, this));
    // LOG_DEBUG << "socket_connect::ctor[" <<  name_ << "] at " << this
    // << " fd=" << sockfd;
    // socket_->setKeepAlive(true);
}

socket_connect::~socket_connect()
{
    // LOG_DEBUG << "socket_connect::dtor[" <<  name_ << "] at " << this
    // << " fd=" << channel_->fd()
    // << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

// bool socket_connect::getTcpInfo(struct tcp_info* tcpi) const
// {
//   return socket_->getTcpInfo(tcpi);
// }

// string socket_connect::getTcpInfoString() const {
//     char buf[1024];
//     buf[0] = '\0';
//     socket_->getTcpInfoString(buf, sizeof buf);
//     return buf;
// }

void socket_connect::send(const void* data, int len) {
    send(std::string(static_cast<const char*>(data), len));
}

void socket_connect::send(const std::string& message)
{
    if (state_ == kConnected) {
        if (loop_->is_in_loop_thread()) {
            send_in_loop(message);
        } else {
            void (socket_connect::*fp)(const std::string& message) = &socket_connect::send_in_loop;
            loop_->run_in_loop(std::bind(fp, this, message));
            //std::forward<string>(message)));
        }
    }
}

// FIXME efficiency!!!
void socket_connect::send(Buffer* buf) {
    if (state_ == kConnected)
    {
    if (loop_->isInLoopThread())
    {
    sendInLoop(buf->peek(), buf->readableBytes());
    buf->retrieveAll();
    }
    else
    {
    void (socket_connect::*fp)(const StringPiece& message) = &socket_connect::sendInLoop;
    loop_->runInLoop(
    std::bind(fp,
    this,     // FIXME
    buf->retrieveAllAsString()));
    //std::forward<string>(message)));
    }
    }
}

void socket_connect::send_in_loop(const std::string& message) {
    send_in_loop(message.data(), message.size());
}

void socket_connect::send_in_loop(const void* data, size_t len) {
    loop_->assert_in_loop_thread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fault_error = false;
    if (state_ == kDisconnected) {
        // LOG_WARN << "disconnected, give up writing";
        return;
    }

    // if no thing in output queue, try writing directly
    if (!channel_->is_writing() && output_buffer_.readable_bytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_callback_) {
                loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
            }
        } else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                // LOG_SYSERR << "socket_connect::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) { // FIXME: any others?
                    fault_error = true;
                }
            }
        }
    }

    assert(remaining <= len);

    if (!fault_error && remaining > 0) {
        size_t old_len = output_buffer_.readable_bytes();
        if (old_len + remaining >= high_water_mark_ && 
            old_len < high_water_mark_ && 
            high_water_mark_callback_) {
            loop_->queue_in_loop(std::bind(high_water_mark_callback_, shared_from_this(), old_len + remaining));
        }
        output_buffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->is_writing()) {
            channel_->enable_writing();
        }
    }
}

void socket_connect::shutdown()
{
  // FIXME: use compare and swap
  if (state_ == kConnected)
  {
    setState(kDisconnecting);
    // FIXME: shared_from_this()?
    loop_->runInLoop(std::bind(&socket_connect::shutdownInLoop, this));
  }
}

void socket_connect::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    // we are not writing
    socket_->shutdownWrite();
  }
}

// void socket_connect::shutdownAndForceCloseAfter(double seconds)
// {
//   // FIXME: use compare and swap
//   if (state_ == kConnected)
//   {
//     setState(kDisconnecting);
//     loop_->runInLoop(std::bind(&socket_connect::shutdownAndForceCloseInLoop, this, seconds));
//   }
// }

// void socket_connect::shutdownAndForceCloseInLoop(double seconds)
// {
//   loop_->assertInLoopThread();
//   if (!channel_->isWriting())
//   {
//     // we are not writing
//     socket_->shutdownWrite();
//   }
//   loop_->runAfter(
//       seconds,
//       makeWeakCallback(shared_from_this(),
//                        &socket_connect::forceCloseInLoop));
// }

void socket_connect::forceClose()
{
  // FIXME: use compare and swap
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    setState(kDisconnecting);
    loop_->queueInLoop(std::bind(&socket_connect::forceCloseInLoop, shared_from_this()));
  }
}

void socket_connect::forceCloseWithDelay(double seconds)
{
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    setState(kDisconnecting);
    loop_->runAfter(
        seconds,
        makeWeakCallback(shared_from_this(),
                         &socket_connect::forceClose));  // not forceCloseInLoop to avoid race condition
  }
}

void socket_connect::forceCloseInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}

const char* socket_connect::stateToString() const
{
  switch (state_)
  {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void socket_connect::setTcpNoDelay(bool on)
{
  socket_->setTcpNoDelay(on);
}

void socket_connect::startRead()
{
  loop_->runInLoop(std::bind(&socket_connect::startReadInLoop, this));
}

void socket_connect::startReadInLoop()
{
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading())
  {
    channel_->enableReading();
    reading_ = true;
  }
}

void socket_connect::stopRead()
{
  loop_->runInLoop(std::bind(&socket_connect::stopReadInLoop, this));
}

void socket_connect::stopReadInLoop()
{
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading())
  {
    channel_->disableReading();
    reading_ = false;
  }
}

void socket_connect::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void socket_connect::connectDestroyed()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected)
  {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void socket_connect::handleRead(Timestamp receiveTime)
{
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
    errno = savedErrno;
    LOG_SYSERR << "socket_connect::handleRead";
    handleError();
  }
}

void socket_connect::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               outputBuffer_.readableBytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disableWriting();
        if (writeCompleteCallback_)
        {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      LOG_SYSERR << "socket_connect::handleWrite";
      // if (state_ == kDisconnecting)
      // {
      //   shutdownInLoop();
      // }
    }
  }
  else
  {
    LOG_TRACE << "Connection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}

void socket_connect::handleClose()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == kConnected || state_ == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  channel_->disableAll();

  socket_connectPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}

void socket_connect::handleError()
{
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "socket_connect::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}


} /* namespace speed::mq */