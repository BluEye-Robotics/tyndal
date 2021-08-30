#include <assert.h>
#include <tyndall/ros_context.h>
#include <tyndall/meta/macro.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef NO_ROS
namespace std_msgs { struct Int32 { int data; }; }
#else
#include <std_msgs/Int32.h>
#endif

using i32 = std_msgs::Int32;

bool operator==(const i32& lhs, const i32& rhs)
{
  return lhs.data == rhs.data;
}

const i32 ref = [](){i32 ret; ret.data = 42; return ret; }();

#define check(cond) do { if (!(cond)){ printf( __FILE__ ":" M_STRINGIFY(__LINE__) " " "Assertion failed: " #cond "\n"); exit(1); }} while(0)

int main()
{
  ros_context::init(0, NULL, std::chrono::milliseconds{3}, "test_ros_context");

  {
    {
      i32 entry = ref;
      ros_context_write(entry, "/test/standard");
    }

    {
      i32 entry = {}, tmp;
      ros_context_read(tmp, "/test/standard");
      sleep(1);
      int rc = ros_context_read(entry, "/test/standard");
      check((rc == 0) || (errno == EAGAIN));
      check(entry == ref);
    }
  }

#ifdef NO_ROS
  ipc_cleanup();
#endif
}
