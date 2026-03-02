#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "table.hpp"
#include "errors.hpp"
#include "model/service.hpp"
#include "model/repository.hpp"
#include "model/entity/program.hpp"
#include "model/entity/unit.hpp"
#include "controller.hpp"
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <future>
#include <chrono>

TEST_CASE("Table: insert, emplace, operator[], at, find, erase, iterators, merge", "[table]")
{
    SECTION("Part 1", "[table]")
    {
        Table<std::string, int> table(5);
        REQUIRE(table.empty());
        REQUIRE(table.size() == 0);

        auto [it, inserted] = table.insert({"a", 1});
        REQUIRE(inserted);
        REQUIRE(table.contains("a"));
        REQUIRE(table.at("a") == 1);

        auto [it1, inserted1] = table.insert({"a", 42});
        REQUIRE(!inserted1);
        REQUIRE(table.at("a") == 1);

        table["b"] = 10;
        REQUIRE(table["b"] == 10);
        REQUIRE(table.size() == 2);

        auto [it2, inserted2] = table.emplace(std::make_pair(std::string("c"), 3));
        REQUIRE(inserted2);
        REQUIRE(table.contains("c"));

        size_t cnt = 0;
        for (auto it_ = table.begin(); it_ != table.end(); ++it_)
            ++cnt;
        REQUIRE(cnt == table.size());

        REQUIRE(table.erase("c") == 1);
        auto itb = table.find("b");
        REQUIRE(itb != table.end());

        auto itnext = table.erase(itb);
        REQUIRE((itnext == table.end() || itnext != itb));
        REQUIRE(table.count("b") == 0);
    }

    SECTION("Part 2", "[table]")
    {
        Table<std::string, int> a(7), b(7);
        a.insert({"k1", 1});
        a.insert({"k2", 2});
        b.insert({"k2", 22});
        b.insert({"k3", 3});

        auto nh = a.extract("k1");
        REQUIRE(!nh.empty());
        b.insert(std::move(nh));
        REQUIRE(b.contains("k1"));
        REQUIRE(b.size() == 3);

        a.insert({"only_a", 9});
        a.merge(b);
        REQUIRE(a.contains("k1"));
        REQUIRE(a.contains("k2"));
        REQUIRE(a.contains("k3"));
        REQUIRE(a.contains("only_a"));

        auto [from1, to1] = a.equal_range("k2");
        REQUIRE(from1 != a.end());

        auto [from2, to2] = a.equal_range("nope");
        REQUIRE(from2 == a.end());
    }

    SECTION("Part 3: rehash/reserve/load factor", "[table]")
    {
        Table<std::string, int> table(2);
        table.insert({"x", 1});
        table.insert({"y", 2});

        float lf_before = table.load_factor();
        REQUIRE(lf_before >= 0.0f);

        table.reserve(100);
        REQUIRE(table.bucket_count() >= 100);

        Table<std::string, int> t2 = table;
        REQUIRE(table == t2);

        t2.insert({"z", 3});
        REQUIRE(table != t2);

        table.swap(t2);
        REQUIRE(table.contains("z"));
        REQUIRE(!t2.contains("z"));

        auto nh = table.extract("no-such");
        REQUIRE(nh.empty());
    }
}

TEST_CASE("Byte and ProgramRepository basic behaviour and exceptions", "[repo][byte]")
{
    ProgramRepository repo(16);
    REQUIRE(!repo[0].isBusy());

    repo[0].grant();
    REQUIRE(repo[0].isBusy());
    REQUIRE_THROWS_AS(repo[0].grant(), std::runtime_error);

    repo[0].write(123);
    REQUIRE(repo[0].read() == 123);

    repo[0].detach();
    REQUIRE(!repo[0].isBusy());
    REQUIRE_THROWS_AS(repo[0].read(), std::logic_error);
    REQUIRE_THROWS_AS(repo[0].write(5), std::logic_error);

    auto prog = std::make_unique<Program>("P", 10);
    repo.addProgram("P", std::move(prog));
    REQUIRE(repo.getPrograms().contains("P"));

    auto prog_dup = std::make_unique<Program>("P", 1);
    REQUIRE_THROWS_AS(repo.addProgram("P", std::move(prog_dup)), std::logic_error);
    REQUIRE_THROWS_AS(repo.getProgram("no"), std::logic_error);

    auto sh = std::make_shared<Shared>("S", 2, 2, 1);
    repo.addShared("S", sh);
    REQUIRE(repo.getShared("S")->getName() == "S");
    REQUIRE_THROWS_AS(repo.addShared("S", sh), std::logic_error);

    repo.removeProgram("P");
    REQUIRE(!repo.getPrograms().contains("P"));
    REQUIRE_THROWS_AS(repo.removeProgram("P"), std::logic_error);

    // Test error handling
    repo.addError("test_prog", std::make_unique<MemoryError>("Test error"));
    REQUIRE(repo.getErrors().contains("test_prog"));
    REQUIRE(!repo.getErrors().at("test_prog").empty());
}

TEST_CASE("Program: createVar/createArr/createRef/getUnit/removeRef/getAccess/detachAccess", "[program]")
{
    Program prog("pr", 50);
    prog.createVar("v", 0, 1);
    REQUIRE(prog.hasOwnUnit("v"));

    prog.createArr("arr", 1, 4, 2);
    REQUIRE(prog.hasOwnUnit("arr"));

    REQUIRE_THROWS_AS(prog.createVar("v", 10, 1), std::logic_error);
    REQUIRE_THROWS_AS(prog.createRef("v", "v"), std::logic_error);

    prog.createRef("v", "rv");
    REQUIRE(prog.hasRef("rv"));

    auto &ref = prog.getRef("rv");
    REQUIRE(ref.getPtr() != nullptr);

    auto &u1 = prog.getUnit("v");
    REQUIRE(u1.getName() == "v");
    REQUIRE(u1.getType() == "Variable");

    REQUIRE_THROWS_AS(prog.getUnit("nope"), IdAccessError);

    prog.removeRef("rv");
    REQUIRE(!prog.hasRef("rv"));
    REQUIRE_THROWS_AS(prog.removeRef("rv"), IdAccessError);

    auto shared = std::make_shared<Shared>("G", 10, 2, 1);
    prog.getAccess(shared);
    REQUIRE(prog.hasShared("G"));

    REQUIRE_THROWS_AS(prog.detachAccess("nope"), IdAccessError);
    prog.detachAccess("G");
    REQUIRE(!prog.hasShared("G"));

    // Test extractOwnUnit
    auto extracted = prog.extractOwnUnit("v");
    REQUIRE(extracted != nullptr);
    REQUIRE(!prog.hasOwnUnit("v"));
    REQUIRE(extracted->getName() == "v");

    // Test hasAnyUnit
    REQUIRE(!prog.hasAnyUnit("v"));
    REQUIRE(prog.hasAnyUnit("arr"));
}

TEST_CASE("MemoryService functionality", "[memoryservice]")
{
    ProgramRepository repo(100);
    MemoryService mem_service(repo);
    SharedService sh_service(mem_service);
    UnitService unit_service(mem_service);
    ProgramService prog_service(mem_service);

    // Test defragmentation
    sh_service.createShared("S", 10, 1);

    prog_service.createProgram("P1", 30);

    unit_service.createVariable("P1", "v1", 5);

    unit_service.createVariable("P1", "v2", 20);

    // Before defragmentation, check positions
    REQUIRE(repo.getProgram("P1").getOwnUnits().at("v2")->getAddress() == 15);

    unit_service.destroyOwnUnit("P1", "v1");

    // Perform defragmentation
    mem_service.defragment();

    // After defragmentation, check new positions
    REQUIRE(repo.getProgram("P1").getOwnUnits().at("v2")->getAddress() == 10);
}

TEST_CASE("ProgramService functionality", "[programservice]")
{
    ProgramRepository repo(200);
    MemoryService mem_service(repo);
    ProgramService prog_service(mem_service);
    UnitService unit_service(mem_service);

    // Test createProgram
    prog_service.createProgram("P1", 20);
    REQUIRE(repo.getPrograms().contains("P1"));

    // Test duplicate program creation
    REQUIRE_THROWS_AS(prog_service.createProgram("P1", 10), std::logic_error);

    // Test getTotalMemoryUsage
    unit_service.createVariable("P1", "v1", 5);
    unit_service.createVariable("P1", "v2", 5);

    REQUIRE(prog_service.getTotalMemoryUsage("P1") == 10);

    // Test getTotalMemoryUsage for non-existent program
    REQUIRE_THROWS_AS(prog_service.getTotalMemoryUsage("non_existent"), std::runtime_error);

    // Test destroyProgram with memory leak
    prog_service.createProgram("P2", 30);
    unit_service.createVariable("P2", "v3", 10);

    REQUIRE_THROWS_AS(prog_service.destroyProgram("P2"), LeakError);
    REQUIRE(repo.getErrors().contains("P2"));

    // Test destroyProgram without leaks
    prog_service.createProgram("P3", 10);
    REQUIRE_NOTHROW(prog_service.destroyProgram("P3"));
    REQUIRE(!repo.getPrograms().contains("P3"));

    // Test multithreaded memory usage calculation
    prog_service.createProgram("P4", 100);

    // Create many units to make multithreading useful
    for (int i = 0; i < 100; ++i)
        unit_service.createVariable("P4", "v" + std::to_string(i), 1);


    auto usage = prog_service.getTotalMemoryUsage("P4");
    REQUIRE(usage == 100); // 100 units of size 1 each

    // Test error handling during program creation
    REQUIRE_THROWS_AS(prog_service.createProgram("P5", 1000), std::logic_error); // Too big for repo size
}

TEST_CASE("SharedService functionality", "[sharedservice]")
{
    ProgramRepository repo(100);
    MemoryService mem_service(repo);
    SharedService sh_service(mem_service);
    ProgramService prog_service(mem_service);

    // Test createShared
    sh_service.createShared("S1", 10, 2);
    REQUIRE(repo.getSharedElems().contains("S1"));

    // Test createShared with invalid parameters
    REQUIRE_THROWS_AS(sh_service.createShared("S2", 5, 3), std::logic_error); // Size not multiple of element size

    // Test duplicate shared element
    REQUIRE_THROWS_AS(sh_service.createShared("S1", 10, 1), std::logic_error);

    // Create programs for testing sharing
    prog_service.createProgram("P1", 20);
    prog_service.createProgram("P2", 20);

    // Test grantShared
    sh_service.grantShared("P1", "S1");
    REQUIRE(repo.getProgram("P1").hasShared("S1"));

    // Test grantShared to non-existent program
    REQUIRE_THROWS_AS(sh_service.grantShared("non_existent", "S1"), std::logic_error);

    // Test grantShared for non-existent shared element
    REQUIRE_THROWS_AS(sh_service.grantShared("P1", "non_existent"), std::logic_error);

    // Test revokeShared
    sh_service.revokeShared("P1", "S1");
    REQUIRE(!repo.getProgram("P1").hasShared("S1"));

    // Test revokeShared for non-accessed shared element
    REQUIRE_THROWS_AS(sh_service.revokeShared("P1", "S1"), IdAccessError);

    // Test destroying shared element accessed by multiple programs
    sh_service.grantShared("P1", "S1");
    sh_service.grantShared("P2", "S1");

    // Test error handling during shared element destruction
    REQUIRE_NOTHROW(sh_service.destroyShared("S1"));
    REQUIRE(!repo.getSharedElems().contains("S1"));
    REQUIRE(!repo.getProgram("P1").hasShared("S1"));
    REQUIRE(!repo.getProgram("P2").hasShared("S1"));
}

TEST_CASE("UnitService functionality", "[unitservice]")
{
    ProgramRepository repo(100);
    MemoryService mem_service(repo);
    UnitService unit_service(mem_service);
    ProgramService prog_service(mem_service);

    // Create a program
    prog_service.createProgram("P1", 30);

    // Test createVariable
    unit_service.createVariable("P1", "v1", 5);
    REQUIRE(repo.getProgram("P1").hasOwnUnit("v1"));

    // Test createVariable for non-existent program
    REQUIRE_THROWS_AS(unit_service.createVariable("non_existent", "v2", 5), std::logic_error);

    // Test getUnit
    const auto& unit = unit_service.getUnit("P1", "v1");
    REQUIRE(unit.getName() == "v1");
    REQUIRE(unit.getTotalSize() == 5);

    // Test getUnit for non-existent unit
    REQUIRE_THROWS_AS(unit_service.getUnit("P1", "non_existent"), IdAccessError);

    // Test readUnit and writeUnit
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    unit_service.writeUnit("P1", "v1", data);

    auto read_data = unit_service.readUnit("P1", "v1");
    REQUIRE(read_data == data);

    // Test writeUnit with wrong size
    std::vector<uint8_t> wrong_data = {1, 2, 3};
    REQUIRE_THROWS_AS(unit_service.writeUnit("P1", "v1", wrong_data), std::logic_error);

    // Test destroyOwnUnit
    unit_service.destroyOwnUnit("P1", "v1");
    REQUIRE(!repo.getProgram("P1").hasOwnUnit("v1"));

    // Test destroyOwnUnit for non-existent unit
    REQUIRE_THROWS_AS(unit_service.destroyOwnUnit("P1", "v1"), IdAccessError);
}

TEST_CASE("IndexedService functionality", "[indexedservice]")
{
    ProgramRepository repo(100);
    MemoryService mem_service(repo);
    UnitService unit_service(mem_service);
    IndexedService idx_service(mem_service, unit_service);
    ProgramService prog_service(mem_service);

    // Create a program
    prog_service.createProgram("P1", 50);

    // Test createArray
    idx_service.createArray("P1", "arr1", 10, 2); // 5 elements of size 2
    REQUIRE(repo.getProgram("P1").hasOwnUnit("arr1"));

    // Test createArray with invalid parameters
    REQUIRE_THROWS_AS(idx_service.createArray("P1", "arr2", 5, 3), std::logic_error); // Size not multiple of element size

    // Test readIndex and writeIndex
    std::vector<uint8_t> elem1 = {10, 20};
    idx_service.writeIndex("P1", "arr1", 0, elem1);

    auto read_elem1 = idx_service.readIndex("P1", "arr1", 0);
    REQUIRE(read_elem1 == elem1);

    // Test readIndex out of bounds
    REQUIRE_THROWS_AS(idx_service.readIndex("P1", "arr1", 5), IndexError);

    // Test writeIndex out of bounds
    std::vector<uint8_t> elem_out = {1, 2};
    REQUIRE_THROWS_AS(idx_service.writeIndex("P1", "arr1", 5, elem_out), IndexError);

    // Test getArrRange
    std::vector<uint8_t> elem2 = {30, 40};
    idx_service.writeIndex("P1", "arr1", 1, elem2);

    auto range = idx_service.getArrRange("P1", "arr1", 0, 2);
    REQUIRE(range.size() == 4);
    REQUIRE(range[0] == 10);
    REQUIRE(range[1] == 20);
    REQUIRE(range[2] == 30);
    REQUIRE(range[3] == 40);

    // Test getShRange
    SharedService sh_service(mem_service);
    sh_service.createShared("S1", 10, 2); // 5 elements of size 2

    // Write to shared element directly
    auto shared = repo.getShared("S1");
    size_t begin = shared->getAddress();
    for (size_t i = 0; i < 10; ++i) {
        repo[begin + i].write(static_cast<uint8_t>(100 + i));
    }

    auto sh_range = idx_service.getShRange("S1", 1, 3);
    REQUIRE(sh_range.size() == 4); // 2 elements * 2 bytes each
    REQUIRE(sh_range[0] == 102);
    REQUIRE(sh_range[1] == 103);
    REQUIRE(sh_range[2] == 104);
    REQUIRE(sh_range[3] == 105);
}

TEST_CASE("RefService functionality", "[refservice]")
{
    ProgramRepository repo(100);
    MemoryService mem_service(repo);
    UnitService unit_service(mem_service);
    RefService ref_service(mem_service);
    ProgramService prog_service(mem_service);

    // Create a program
    prog_service.createProgram("P1", 30);
    auto& prog = repo.getProgram("P1");

    // Create a variable to reference
    unit_service.createVariable("P1", "v1", 5);

    // Test createRef
    ref_service.createRef("P1", "v1", "ref1");
    REQUIRE(prog.hasRef("ref1"));

    // Test createRef to non-existent unit
    REQUIRE_THROWS_AS(ref_service.createRef("P1", "non_existent", "ref2"), std::logic_error);

    // Test createRef with duplicate name
    REQUIRE_THROWS_AS(ref_service.createRef("P1", "v1", "ref1"), std::logic_error);

    // Test createRef with same name as target
    REQUIRE_THROWS_AS(ref_service.createRef("P1", "v1", "v1"), std::logic_error);

    // Test removeRef
    ref_service.removeRef("P1", "ref1");
    REQUIRE(!prog.hasRef("ref1"));

    // Test removeRef for non-existent ref
    REQUIRE_THROWS_AS(ref_service.removeRef("P1", "ref1"), IdAccessError);

    // Test reference after removing original unit
    ref_service.createRef("P1", "v1", "ref2");
    unit_service.destroyOwnUnit("P1", "v1");

    // Reference should still exist but be detached
    REQUIRE(prog.hasRef("ref2"));
    REQUIRE(prog.getRef("ref2").detached());
}

TEST_CASE("Controller integration tests", "[controller][integration]")
{
    ProgramRepository repo(200);
    MemoryService mem_service(repo);
    ProgramService prog_service(mem_service);
    SharedService sh_service(mem_service);
    UnitService unit_service(mem_service);
    IndexedService idx_service(mem_service, unit_service);
    RefService ref_service(mem_service);

    Controller controller(repo, mem_service, prog_service, sh_service, idx_service, unit_service, ref_service);

    // Test program creation and destruction
    controller.createProgram("P1", 50);
    REQUIRE(repo.getPrograms().contains("P1"));

    // Test variable creation and access
    controller.createVariable("P1", "var1", 10);
    REQUIRE(repo.getProgram("P1").hasOwnUnit("var1"));

    // Test variable reading and writing
    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    controller.writeUnit("P1", "var1", data);

    auto read_data = controller.readUnit("P1", "var1");
    REQUIRE(read_data == data);

    // Test array creation and indexing
    controller.createArray("P1", "arr1", 20, 4); // 5 elements of 4 bytes each

    std::vector<uint8_t> elem = {10, 20, 30, 40};
    controller.writeIndex("P1", "arr1", 0, elem);

    auto read_elem = controller.readIndex("P1", "arr1", 0);
    REQUIRE(read_elem == elem);

    // Test array range reading
    auto range = controller.getArrRange("P1", "arr1", 0, 2);
    REQUIRE(range.size() == 8); // 2 elements * 4 bytes

    // Test shared element creation and access
    controller.createShared("shared1", 30, 5); // 6 elements of 5 bytes each

    controller.createProgram("P2", 40);
    controller.grantShared("P2", "shared1");

    REQUIRE(repo.getProgram("P2").hasShared("shared1"));

    // Test shared element range reading
    auto sh_range = controller.getShRange("shared1", 1, 3);
    REQUIRE(sh_range.size() == 10); // 2 elements * 5 bytes

    // Test reference creation and removal
    controller.createRef("P1", "var1", "ref1");
    REQUIRE(repo.getProgram("P1").hasRef("ref1"));

    controller.removeRef("P1", "ref1");
    REQUIRE(!repo.getProgram("P1").hasRef("ref1"));

    // Test defragmentation
    controller.defragment();

    // Test total memory usage calculation
    auto usage = controller.getTotalMemoryUsage("P1");
    REQUIRE(usage > 0);

    // Test error handling - destroying program with memory leak
    controller.createVariable("P1", "var2", 5);
    REQUIRE_THROWS_AS(controller.destroyProgram("P1"), LeakError);

    controller.revokeShared("P2", "shared1");
    controller.destroyProgram("P2");
}

TEST_CASE("Multithreaded operations", "[threading]")
{
    ProgramRepository repo(1000);
    MemoryService mem_service(repo);
    ProgramService prog_service(mem_service);
    UnitService unit_service(mem_service);

    // Create a program with large quota
    prog_service.createProgram("big_prog", 500);

    // Create many variables in parallel
    std::vector<std::thread> threads;
    for (int i = 0; i < 50; ++i) {
        threads.emplace_back([&, i]() {
            try {
                unit_service.createVariable("big_prog", "var" + std::to_string(i), 5);
            } catch (const std::exception& e) {
                repo.addError("big_prog", std::make_unique<MemoryError>(e.what()));
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Verify all variables were created
    auto& prog = repo.getProgram("big_prog");
    REQUIRE(prog.getOwnUnits().size() == 50);

    // Read memory usage in parallel
    std::vector<std::future<size_t>> futures;
    for (int i = 0; i < 4; ++i) {
        futures.push_back(std::async(std::launch::async, [&]() {
            return prog_service.getTotalMemoryUsage("big_prog");
        }));
    }

    for (auto& future : futures) {
        auto usage = future.get();
        REQUIRE(usage == 250); // 50 variables * 5 bytes each
    }
}