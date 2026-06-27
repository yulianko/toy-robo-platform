#pragma once
#include "MotionSequence.h"

struct MotionCommand {
    enum class Type : uint8_t {
        Play,
        Stop,
        Brake,
    };

    Type type = Type::Stop;
    MotionSequence sequence;

    static MotionCommand play(const MotionSequence& seq) {
        MotionCommand cmd;
        cmd.type = Type::Play;
        cmd.sequence = seq;
        return cmd;
    }

    static MotionCommand stop() {
        MotionCommand cmd;
        cmd.type = Type::Stop;
        return cmd;
    }

    static MotionCommand brake() {
        MotionCommand cmd;
        cmd.type = Type::Brake;
        return cmd;
    }
};
