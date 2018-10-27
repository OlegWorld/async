#include <iostream>
#include <fstream>
#include <sstream>
#include <experimental/filesystem>
#include <gtest/gtest.h>
#include "async.h"
#include "command_handler.h"

TEST(AsyncTest, CommandBulkTest) {
    CommandBulk b;
    EXPECT_TRUE(b.empty());
    EXPECT_FALSE(b.recyclable());

    b.push_command("command1");
    b.push_command("command2");
    EXPECT_FALSE(b.empty());
    EXPECT_EQ(b.size(), 2);
    EXPECT_FALSE(b.recyclable());

    testing::internal::CaptureStdout();
    std::cout << b;
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command1, command2\n");

    b.clear();
    EXPECT_TRUE(b.empty());
    EXPECT_EQ(b.size(), 0);
    EXPECT_TRUE(b.recyclable());
}

TEST(AsyncTest, ReaderNormalSingleTest) {
    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\n");
    CommandReader reader(data, 1);
    reader.scan_input(inputString);
    EXPECT_FALSE(data.empty());

    auto& b1 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b1;
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command1\n");
    output.clear();

    data.pop_front();
    EXPECT_FALSE(data.empty());

    auto& b2 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b2;
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command2\n");

    data.pop_front();
    EXPECT_TRUE(data.empty());
}

TEST(AsyncTest, ReaderNormalDoubleTest) {
    std::list<CommandBulk> data;
    std::string inputString("command1\ncommand2\ncommand3\n");
    CommandReader reader(data, 2);
    reader.scan_input(inputString);
    EXPECT_FALSE(data.empty());

    auto& b1 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b1;
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command1, command2\n");
    output.clear();

    data.pop_front();
    EXPECT_FALSE(data.empty());

    inputString = "command4\ncommand5\ncommand6\n";
    reader.scan_input(inputString);
    EXPECT_FALSE(data.empty());

    auto& b2 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b2;
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command3, command4\n");

    data.pop_front();
    EXPECT_FALSE(data.empty());

    auto& b3 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b3;
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command5, command6\n");

    data.pop_front();
    EXPECT_TRUE(data.empty());
}

TEST(AsyncTest, ReaderSwitchDoubleTest) {
    std::list<CommandBulk> data;
    std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    CommandReader reader(data, 2);
    reader.scan_input(inputString);
    EXPECT_FALSE(data.empty());

    auto& b1 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b1;
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command1, command2\n");
    output.clear();

    data.pop_front();
    EXPECT_FALSE(data.empty());

    auto& b2 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b2;
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command3\n");
    output.clear();

    data.pop_front();
    EXPECT_FALSE(data.empty());

    auto& b3 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b3;
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command4, command5, command6\n");
    output.clear();

    data.pop_front();
    EXPECT_FALSE(data.empty());

    auto& b4 = data.front();
    testing::internal::CaptureStdout();
    std::cout << b4;
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command7\n");
    output.clear();

    data.pop_front();
    EXPECT_TRUE(data.empty());
}

TEST(AsyncTest, ReaderProcessorSwitchDoubleTest) {
    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    CommandProcessor processor;
    CommandReader reader(data, 2);
    reader.subscribe(&processor);

    while(!processor.ready());
    testing::internal::CaptureStdout();
    reader.scan_input(inputString);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command1, command2\nbulk: command3\nbulk: command4, command5, command6\n");
    processor.stop();
}

TEST(AsyncTest, ReaderSingleLogSwitchDoubleTest) {
    namespace fs = std::experimental::filesystem;

    fs::remove_all("log");

    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    CommandMultipleLog log(1);
    CommandReader reader(data, 2);
    reader.subscribe(&log);

    while(!log.ready());
    reader.scan_input(inputString);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 3);
    log.stop();
}

TEST(AsyncTest, ReaderDoubleLogSwitchDoubleTest) {
    namespace fs = std::experimental::filesystem;
    fs::remove_all("log");

    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    CommandMultipleLog log(2);
    CommandReader reader(data, 2);
    reader.subscribe(&log);

    while(!log.ready());
    reader.scan_input(inputString);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 3);
    log.stop();
}

TEST(AsyncTest, CommandHandlerTest) {
    namespace fs = std::experimental::filesystem;
    fs::remove_all("log");

    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");

    CommandHandler handler(2, 1);

    testing::internal::CaptureStdout();
    handler.read(inputString);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::string output = testing::internal::GetCapturedStdout();

    std::string ref_output = "bulk: command1, command2\n"
                             "bulk: command3\n"
                             "bulk: command4, command5, command6\n";

    EXPECT_EQ(output, ref_output);

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 3);
}

TEST(AsyncTest, MultipleHandlerTest) {
    namespace fs = std::experimental::filesystem;
    fs::remove_all("log");

    std::size_t bulk = 5;
    auto h = async::connect(bulk);
    auto h2 = async::connect(bulk);

    testing::internal::CaptureStdout();
    async::receive(h, "1", 1);
    async::receive(h2, "1\n", 2);
    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
    async::receive(h, "b\nc\nd\n}\n89\n", 11);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::string output = testing::internal::GetCapturedStdout();
    std::string ref_output = "bulk: 1, 2, 3, 4, 5\n"
                             "bulk: 6\n"
                             "bulk: a, b, c, d\n";
    EXPECT_EQ(output, ref_output);
    async::disconnect(h);
    async::disconnect(h2);

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 3);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

