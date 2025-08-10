# ROP Gadget Simulation

## Overview

This project provides an educational simulation of Return-Oriented Programming (ROP) attacks using gadgets. The simulation demonstrates how attackers can chain together small code fragments (gadgets) from existing legitimate code to execute arbitrary operations without injecting new code into memory. This technique is commonly used to bypass Data Execution Prevention (DEP) and other modern security protections.

## Build & Run

```bash
# Clone the repository
git clone https://github.com/AniDashyan/return_oriented_programming
cd return_oriented_programming

# Build the project
cmake -S . -B build
cmake --build build

# Run the executable
./build/main
```

## Example Output
```
=== Simple ROP Gadget Demo ===

=== How gadgets are found ===
Scanning binary for instruction sequences ending in RET:

0x401230: mov eax, ebx
0x401232: push ecx
0x401234: pop rax        <- Gadget 1 starts here
0x401235: ret            <- Ends with RET

0x401565: xor eax, eax
0x401567: add rax, rbx   <- Gadget 2
0x401569: ret

0x401888: push rdi
0x401890: mov [rbx], rax <- Gadget 3
0x401892: ret

=== Setting up ROP chain ===
Buffer overflow overwrites stack with gadget addresses

ROP chain on stack:
  stack[80] = 0x401234
  stack[81] = 0xdeadbeef
  stack[82] = 0x401567
  stack[83] = 0x401890
  stack[84] = 0x0

=== Executing ROP chain ===
RAX: 0x0  RBX: 0x600000
RSP: 80  RIP: 0x401234
Next stack value: 0xdeadbeef

Executing: pop rax; ret
  Popped 0xdeadbeef into RAX
  Returning to 0x401567

RAX: 0xdeadbeef  RBX: 0x600000
RSP: 82  RIP: 0x401567
Next stack value: 0x401890

Executing: add rax, rbx; ret
  0xdeadbeef + 0x600000 = 0xdeb3beef
  Returning to 0x401890

RAX: 0xdeb3beef  RBX: 0x600000
RSP: 83  RIP: 0x401890
Next stack value: 0x0

Executing: mov [rbx], rax; ret
  Writing 0xdeb3beef to memory at 0x600000
  Returning to 0x0

RAX: 0xdeb3beef  RBX: 0x600000
RSP: 84  RIP: 0x0
Next stack value: 0x0

ROP chain complete!
```

## How Does It Work?

### What is Return-Oriented Programming (ROP)?

Return-Oriented Programming is an advanced exploitation technique that allows attackers to execute arbitrary code without injecting new instructions into memory. Instead, it reuses existing code fragments called "gadgets" from legitimate executable sections of a program or its libraries.

### Why is ROP Used?

ROP is primarily used to bypass modern security protections:

- **Data Execution Prevention (DEP/NX bit)**: Prevents execution of code in data sections
- **Code injection detection**: Traditional shellcode injection is blocked
- **Write XOR Execute**: Memory pages cannot be both writable and executable

Since ROP uses existing executable code, it bypasses these protections by design.

### How This Simulation Works

#### 1. CPU State Simulation

The project simulates a simplified CPU with essential components:

```cpp
struct CPU {
    unsigned long rax, rbx, rcx, rdx;  // General-purpose registers
    unsigned long rsp, rip;            // Stack pointer, instruction pointer
    unsigned long stack[100];          // Simulated memory stack
};
```

This structure tracks:
- **Registers**: Store intermediate values during computation
- **RSP (Stack Pointer)**: Points to current position in the stack
- **RIP (Instruction Pointer)**: Address of next instruction to execute
- **Stack**: Memory where the ROP chain is constructed

#### 2. Gadget Implementation

Each gadget represents a small sequence of assembly instructions ending with `ret`:

```cpp
void gadget_pop_rax(CPU* cpu) {
    // Simulate: pop rax; ret
    cpu->rax = cpu->stack[cpu->rsp];  // Pop value into RAX
    cpu->rsp++;                       // Advance stack pointer
    cpu->rip = cpu->stack[cpu->rsp];  // RET: get next address
    cpu->rsp++;                       // Advance stack pointer
}
```

**Key aspects:**
- **POP operation**: Takes data from stack into register
- **RET instruction**: Transfers control to next gadget address
- **Stack manipulation**: Each operation advances the stack pointer

#### 3. ROP Chain Construction

The attack payload is constructed as a sequence of addresses on the stack:

```cpp
cpu->stack[80] = 0x401234;    // Address: pop rax; ret
cpu->stack[81] = 0xDEADBEEF;  // Data: value to pop into RAX
cpu->stack[82] = 0x401567;    // Address: add rax, rbx; ret
cpu->stack[83] = 0x401890;    // Address: mov [rbx], rax; ret
cpu->stack[84] = 0x0;         // End marker
```

**Execution flow:**
1. Function returns to first gadget address
2. First gadget executes, then "returns" to second gadget
3. Process continues until the entire payload executes
4. Each `ret` instruction chains to the next gadget

#### 4. Attack Simulation

The main execution loop demonstrates how ROP chains execute:

```cpp
while (cpu->rip != 0) {
    if (cpu->rip == 0x401234) {
        gadget_pop_rax(cpu);
    } else if (cpu->rip == 0x401567) {
        gadget_add_rax_rbx(cpu);
    }
    // ... more gadgets
}
```

**This simulates:**
- **Buffer overflow**: Stack is controlled by attacker
- **Code reuse**: Only existing code addresses are used
- **Arbitrary computation**: Complex operations via gadget chaining

### Real-World Impact

In actual exploits, ROP enables:
- **Privilege escalation**: Calling system functions with elevated permissions
- **Code execution**: Running shellcode despite DEP protection
- **Sandbox escape**: Bypassing application security boundaries
- **Memory corruption**: Writing to protected memory regions

### Defense Mechanisms

Modern systems employ several defenses against ROP:
- **ASLR (Address Space Layout Randomization)**: Randomizes gadget locations
- **CFI (Control Flow Integrity)**: Restricts valid return targets
- **Intel CET**: Hardware-level return address verification
- **Stack canaries**: Detect stack corruption before return

This simulation helps understand both the attack vector and why these defensive measures are necessary for modern system security.
