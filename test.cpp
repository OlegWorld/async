#include <iostream>
#include <fstream>
#include <sstream>
#include <experimental/filesystem>
#include <gtest/gtest.h>
#include "command_handler.h"

TEST(BulkMTTest, CommandBulkTest) {
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

TEST(BulkMTTest, ReaderNormalSingleTest) {
    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\n");
    std::istringstream inputStream(inputString);
    CommandReader reader(data, 1, inputStream);
    reader.scan_input();
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
    EXPECT_FALSE(data.empty());

    auto& b3 = data.front();
    EXPECT_TRUE(b3.empty());
    EXPECT_TRUE(b3.recyclable());

    data.pop_front();
    EXPECT_TRUE(data.empty());

    EXPECT_EQ(reader.lines(), 2);
    EXPECT_EQ(reader.commands(), 2);
    EXPECT_EQ(reader.blocks(), 2);
}

TEST(BulkMTTest, ReaderNormalDoubleTest) {
    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n");
    std::istringstream inputStream(inputString);
    CommandReader reader(data, 2, inputStream);
    reader.scan_input();
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

    data.pop_front();
    EXPECT_TRUE(data.empty());

    EXPECT_EQ(reader.lines(), 3);
    EXPECT_EQ(reader.commands(), 3);
    EXPECT_EQ(reader.blocks(), 2);
}

TEST(BulkMTTest, ReaderSwitchDoubleTest) {
    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    std::istringstream inputStream(inputString);
    CommandReader reader(data, 2, inputStream);
    reader.scan_input();
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

    EXPECT_EQ(reader.lines(), 9);
    EXPECT_EQ(reader.commands(), 7);
    EXPECT_EQ(reader.blocks(), 4);
}

TEST(BulkMTTest, ReaderProcessorSwitchDoubleTest) {
    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    std::istringstream inputStream(inputString);
    CommandProcessor processor;
    CommandReader reader(data, 2, inputStream);
    reader.subscribe(&processor);

    while(!processor.ready());
    testing::internal::CaptureStdout();
    reader.scan_input();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "bulk: command1, command2\nbulk: command3\nbulk: command4, command5, command6\nbulk: command7\n");

    EXPECT_EQ(reader.lines(), 9);
    EXPECT_EQ(reader.commands(), 7);
    EXPECT_EQ(reader.blocks(), 4);

    EXPECT_EQ(processor.blocks(), 4);
    EXPECT_EQ(processor.commands(), 7);
}

TEST(BulkMTTest, ReaderSingleLogSwitchDoubleTest) {
    namespace fs = std::experimental::filesystem;

    fs::remove_all("log");

    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    std::istringstream inputStream(inputString);
    CommandMultipleLog log(1);
    CommandReader reader(data, 2, inputStream);
    reader.subscribe(&log);

    while(!log.ready());
    reader.scan_input();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(reader.lines(), 9);
    EXPECT_EQ(reader.commands(), 7);
    EXPECT_EQ(reader.blocks(), 4);

    EXPECT_EQ(log.in_blocks(), 4);
    EXPECT_EQ(log.out_blocks(0), 4);
    EXPECT_EQ(log.commands(0), 7);

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 4);
}

TEST(BulkMTTest, ReaderDoubleLogSwitchDoubleTest) {
    namespace fs = std::experimental::filesystem;
    fs::remove_all("log");

    std::list<CommandBulk> data;
    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    std::istringstream inputStream(inputString);
    CommandMultipleLog log(2);
    CommandReader reader(data, 2, inputStream);
    reader.subscribe(&log);

    while(!log.ready());
    reader.scan_input();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(reader.lines(), 9);
    EXPECT_EQ(reader.commands(), 7);
    EXPECT_EQ(reader.blocks(), 4);

    EXPECT_EQ(log.in_blocks(), 4);
    EXPECT_EQ(log.out_blocks(0) + log.out_blocks(1), 4);
    EXPECT_EQ(log.commands(0) + log.commands(1), 7);

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 4);
}

TEST(BulkMTTest, CommandHandlerTest) {
    namespace fs = std::experimental::filesystem;
    fs::remove_all("log");

    const std::string inputString("command1\ncommand2\ncommand3\n{\ncommand4\ncommand5\ncommand6\n}\ncommand7\n");
    std::istringstream inputStream(inputString);

    testing::internal::CaptureStdout();
    CommandHandler(2, 1, inputStream);
    std::string output = testing::internal::GetCapturedStdout();
    std::string ref_output = "bulk: command1, command2\n"
                             "bulk: command3\n"
                             "bulk: command4, command5, command6\n"
                             "bulk: command7\n";
    std::string ref_finish = "main thread - 9 lines, 7 commands, 4 blocks\n"
                             "log thread - 4 blocks, 7 commands\n"
                             "file1 thread - 4 blocks, 7 commands\n";
    EXPECT_EQ(output, ref_output + ref_finish);

    size_t counter = 0;
    for (const auto & p : fs::directory_iterator(fs::current_path())) {
        counter++;
    }

    EXPECT_EQ(counter, 4);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

