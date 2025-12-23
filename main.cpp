#include <iostream>
#include <cstdio> // For printf

#define NUM_REGS 8

// --- 1. ARCHITECTURE DEFINITIONS ---

typedef enum {
    OP_MOV_IMM,   // R1 = Value
    OP_ADD_REG,   // R1 = R1 + R2
    OP_ADD_IMM,   // R1 = R1 + Value
    OP_TRAP,      // System Call
    OP_HALT       // Stop Simulation
} Opcode;

typedef struct {
    Opcode opcode;
    int r1;         // Target Register

    // The Union: "Context Sensitive Argument"
    union {
        int reg_index;   // Used for ADD_REG
        int imm_value;   // Used for MOV_IMM, ADD_IMM
    } arg2;
} Instruction;

typedef enum {
    KERNEL_MODE,
    USER_MODE
} ExecutionMode;

typedef struct {
    int regs[NUM_REGS];
    unsigned int pc;
    ExecutionMode mode;
    int interrupts_enabled;
} CPU;

// --- 2. THE CPU ENGINE ---

// Returns 1 if CPU should keep running, 0 if HALT
int cpu_step(CPU *ptr, Instruction Memory[]) {

    // 1. FETCH
    Instruction instr = Memory[ptr->pc];

    // 2. DECODE & EXECUTE
    switch (instr.opcode) {
        case OP_MOV_IMM: {
            int target_reg = instr.r1;
            int value = instr.arg2.imm_value;
            ptr->regs[target_reg] = value;
            break;
        }

        case OP_ADD_REG: {
            int target_reg = instr.r1;
            int source_reg = instr.arg2.reg_index;
            int source_value = ptr->regs[source_reg];
            ptr->regs[target_reg] += source_value;
            break;
        }

        case OP_ADD_IMM: {
            int target_reg = instr.r1;
            int value = instr.arg2.imm_value;
            ptr->regs[target_reg] += value;
            break;
        }

        case OP_TRAP: {
            printf("[HARDWARE] --- TRAP TRIGGERED! Switching to KERNEL MODE ---\n");
            ptr->mode = KERNEL_MODE;
            // In a real OS, we would jump to a handler address here
            break;
        }

        case OP_HALT: {
            printf("[HARDWARE] --- HALT DETECTED. Stopping CPU. ---\n");
            return 0; // Return 0 to stop the loop
        }
    }

    // 3. ADVANCE
    ptr->pc++;
    return 1; // Return 1 to keep going
}

// --- 3. MAIN (The Motherboard) ---

int main() {
    // A. Initialize Hardware
    CPU cpu = {0}; // Zero out registers and PC
    cpu.mode = USER_MODE; // Start in User Mode

    Instruction memory[1024]; // 1KB of Instruction Memory

    // B. Load "Test Program" into Memory manually
    // Program:
    // 0: MOV R0, 10    (R0 = 10)
    // 1: ADD R0, 5     (R0 = 10 + 5 = 15)
    // 2: TRAP          (Switch to Kernel)
    // 3: HALT          (Stop)

    // Instruction 0: MOV R0, 10
    memory[0].opcode = OP_MOV_IMM;
    memory[0].r1 = 0;
    memory[0].arg2.imm_value = 10;

    // Instruction 1: ADD R0, 5
    memory[1].opcode = OP_ADD_IMM;
    memory[1].r1 = 0;
    memory[1].arg2.imm_value = 5;

    // Instruction 2: TRAP
    memory[2].opcode = OP_TRAP;

    // Instruction 3: HALT
    memory[3].opcode = OP_HALT;

    // C. The Simulation Loop (Fetch-Decode-Execute Cycle)
    printf("Starting Simulation...\n");

    int running = 1;
    while (running) {
        // [Debug] Show current state before execution
        printf("PC: %d | R0: %d | Mode: %s\n",
               cpu.pc,
               cpu.regs[0],
               (cpu.mode == KERNEL_MODE) ? "KERNEL" : "USER");

        // Execute one step
        running = cpu_step(&cpu, memory);

        // [Optional] Sleep or wait here to slow it down if you want
    }

    printf("Simulation Finished.\n");
    return 0;
}