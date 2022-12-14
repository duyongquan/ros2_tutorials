#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <set>
#include <string>

#include "geometry_msgs/msg/pose_stamped.hpp"

#include "../../test_behavior_tree_fixture.hpp"
#include "behavior_tree/plugins/condition/goal_reached_condition.hpp"

using namespace std::chrono;  // NOLINT
using namespace std::chrono_literals;  // NOLINT

class GoalReachedConditionTestFixture : public behavior_tree::BehaviorTreeTestFixture
{
public:
  void SetUp()
  {
    node_->declare_parameter("transform_tolerance", rclcpp::ParameterValue{0.1});

    geometry_msgs::msg::PoseStamped goal;
    goal.header.stamp = node_->now();
    goal.header.frame_id = "map";
    goal.pose.position.x = 1.0;
    goal.pose.position.y = 1.0;
    config_->blackboard->set("goal", goal);

    std::string xml_txt =
      R"(
      <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <GoalReached goal="{goal}" />
        </BehaviorTree>
      </root>)";

    factory_->registerNodeType<ros2_tutorials::behavior_tree::GoalReachedCondition>("GoalReached");
    tree_ = std::make_shared<BT::Tree>(factory_->createTreeFromText(xml_txt, config_->blackboard));
  }

  void TearDown()
  {
    tree_.reset();
  }

protected:
  static std::shared_ptr<BT::Tree> tree_;
};

std::shared_ptr<BT::Tree> GoalReachedConditionTestFixture::tree_ = nullptr;

TEST_F(GoalReachedConditionTestFixture, test_behavior)
{
  EXPECT_EQ(tree_->tickRoot(), BT::NodeStatus::FAILURE);

  geometry_msgs::msg::Pose pose;
  pose.position.x = 0.0;
  pose.position.y = 0.0;
  transform_handler_->updateRobotPose(pose);
  std::this_thread::sleep_for(500ms);
  EXPECT_EQ(tree_->tickRoot(), BT::NodeStatus::FAILURE);

  pose.position.x = 0.5;
  pose.position.y = 0.5;
  transform_handler_->updateRobotPose(pose);
  std::this_thread::sleep_for(500ms);
  EXPECT_EQ(tree_->tickRoot(), BT::NodeStatus::FAILURE);

  pose.position.x = 0.9;
  pose.position.y = 0.9;
  transform_handler_->updateRobotPose(pose);
  std::this_thread::sleep_for(500ms);
  EXPECT_EQ(tree_->tickRoot(), BT::NodeStatus::SUCCESS);

  pose.position.x = 1.0;
  pose.position.y = 1.0;
  transform_handler_->updateRobotPose(pose);
  std::this_thread::sleep_for(500ms);
  EXPECT_EQ(tree_->tickRoot(), BT::NodeStatus::SUCCESS);
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  // initialize ROS
  rclcpp::init(argc, argv);

  bool all_successful = RUN_ALL_TESTS();

  // shutdown ROS
  rclcpp::shutdown();

  return all_successful;
}