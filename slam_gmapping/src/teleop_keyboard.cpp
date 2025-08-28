#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "std_msgs/msg/header.hpp"
#include "ncurses.h" // 使用 ncurses 库处理键盘输入

using namespace std::chrono_literals;

class TeleopTwistStamped : public rclcpp::Node
{
public:
  TeleopTwistStamped()
  : Node("teleop_keyboard"), linear_(0), angular_(0)
  {
    // 创建发布者，发布 TwistStamped 消息
    publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("/cmd_vel", 10);
    
    // 初始化 ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    timeout(0);
    keypad(stdscr, TRUE);
    
    // 打印使用说明
    printw("Teleop Twist Stamped Keyboard Controller");
    printw("---------------------------");
    printw("Moving around:");
    printw("   w    ");
    printw("a   d");
    printw("   x    ");
    printw("");
    printw("w/x : increase/decrease linear velocity");
    printw("a/d : increase/decrease angular velocity");
    printw("s : force stop");
    printw("CTRL-C to quit");
    printw("---------------------------");
    printw("Current: linear=%.2f, angular=%.2f", linear_, angular_);
    refresh();
    
    // 创建定时器，定期发布消息
    timer_ = this->create_wall_timer(
      100ms, std::bind(&TeleopTwistStamped::timer_callback, this));
  }

  ~TeleopTwistStamped()
  {
    // 清理 ncurses
    endwin();
  }

  void process_keyboard()
  {
    int c = getch();
    
    switch(c)
    {
      case 'w':
        linear_ += 0.1;
        break;
      case 'x':
        linear_ -= 0.1;
        break;
      case 'a':
        angular_ += 0.1;
        break;
      case 'd':
        angular_ -= 0.1;
        break;
      case 's':
        linear_ = 0.0;
        angular_ = 0.0;
        break;
      default:
        // 无有效按键，保持当前速度
        break;
    }
    
    // 限制速度范围
    linear_ = std::max(std::min(linear_, 1.0), -1.0);
    angular_ = std::max(std::min(angular_, 1.0), -1.0);
    
    // 更新显示
    move(10, 0);
    clrtoeol();
    printw("Current: linear=%.2f, angular=%.2f", linear_, angular_);
    refresh();
  }

private:
  void timer_callback()
  {
    process_keyboard();
    
    auto message = geometry_msgs::msg::TwistStamped();
    
    // 设置 header
    message.header.stamp = this->now();
    message.header.frame_id = "base_link"; // 根据你的机器人配置调整
    
    // 设置速度
    message.twist.linear.x = linear_;
    message.twist.angular.z = angular_;
    
    // 发布消息
    publisher_->publish(message);
  }
  
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr publisher_;
  double linear_;
  double angular_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<TeleopTwistStamped>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
