const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "clox",
        .target = target,
        .optimize = optimize
    });

    exe.root_module.addCSourceFiles(.{
        .files = &.{
            "src/main.c",
            "src/chunk.c",
            "src/compiler.c",
            "src/debug.c",
            "src/memory.c",
            "src/object.c",
            "src/scanner.c",
            "src/table.c",
            "src/value.c",
            "src/vm.c"
        }, .flags = &.{
            "-std=c99",
            "-Wall",
            "-Wextra",
            "-Wno-unused-function",
            "-Wno-unused-parameter"
        } });

    exe.linkLibC();
    exe.linkSystemLibrary("readline");

    b.installArtifact(exe);

    const runCmd = b.addRunArtifact(exe);
    runCmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        runCmd.addArgs(args);
    }

    const runStep = b.step("run", "Run the Lox VM");
    runStep.dependOn(&runCmd.step);
}