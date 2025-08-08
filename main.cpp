#include <iostream>
#include <iomanip>

// Simple CPU state structure
struct CPU {
    unsigned long rax, rbx, rcx, rdx;
    unsigned long rsp, rip;
    unsigned long stack[100];
};

// Initialize CPU state
void init_cpu(CPU* cpu) {
    cpu->rax = 0;
    cpu->rbx = 0x600000;  // Target memory address
    cpu->rcx = 0;
    cpu->rdx = 0;
    cpu->rsp = 80;  // Start stack pointer at index 80
    cpu->rip = 0;
    
    // Clear stack
    for (int i = 0; i < 100; i++) {
        cpu->stack[i] = 0;
    }
}

// Print CPU state
void print_cpu(CPU* cpu) {
    std::cout << "RAX: 0x" << std::hex << cpu->rax << "  RBX: 0x" << cpu->rbx << "\n";
    std::cout << "RSP: " << std::dec << cpu->rsp << "  RIP: 0x" << std::hex << cpu->rip << "\n";
    std::cout << "Next stack value: 0x" << cpu->stack[cpu->rsp] << std::dec << "\n\n";
}

// Gadget 1: pop rax; ret
void gadget_pop_rax(CPU* cpu) {
    std::cout << "Executing: pop rax; ret\n";
    
    // Pop value into RAX
    cpu->rax = cpu->stack[cpu->rsp];
    cpu->rsp++;
    std::cout << "  Popped 0x" << std::hex << cpu->rax << std::dec << " into RAX\n";
    
    // RET - jump to next address on stack
    cpu->rip = cpu->stack[cpu->rsp];
    cpu->rsp++;
    std::cout << "  Returning to 0x" << std::hex << cpu->rip << std::dec << "\n\n";
}

// Gadget 2: add rax, rbx; ret  
void gadget_add_rax_rbx(CPU* cpu) {
    std::cout << "Executing: add rax, rbx; ret\n";
    
    unsigned long old_rax = cpu->rax;
    cpu->rax += cpu->rbx;
    std::cout << "  0x" << std::hex << old_rax << " + 0x" << cpu->rbx;
    std::cout << " = 0x" << cpu->rax << std::dec << "\n";
    
    // RET - jump to next address
    cpu->rip = cpu->stack[cpu->rsp];  
    cpu->rsp++;
    std::cout << "  Returning to 0x" << std::hex << cpu->rip << std::dec << "\n\n";
}

// Gadget 3: mov [rbx], rax; ret
void gadget_mov_mem(CPU* cpu) {
    std::cout << "Executing: mov [rbx], rax; ret\n";
    std::cout << "  Writing 0x" << std::hex << cpu->rax;
    std::cout << " to memory at 0x" << cpu->rbx << std::dec << "\n";
    
    // RET - jump to next address
    cpu->rip = cpu->stack[cpu->rsp];
    cpu->rsp++;
    std::cout << "  Returning to 0x" << std::hex << cpu->rip << std::dec << "\n\n";
}

// Set up the ROP chain on the stack
void setup_rop_chain(CPU* cpu) {
    std::cout << "=== Setting up ROP chain ===\n";
    std::cout << "Buffer overflow overwrites stack with gadget addresses\n\n";
    
    // ROP chain: addresses and data on stack
    cpu->stack[80] = 0x401234;    // pop rax; ret
    cpu->stack[81] = 0xDEADBEEF;  // value to pop into rax
    cpu->stack[82] = 0x401567;    // add rax, rbx; ret
    cpu->stack[83] = 0x401890;    // mov [rbx], rax; ret  
    cpu->stack[84] = 0x0;         // end (return to 0)
    
    std::cout << "ROP chain on stack:\n";
    for (int i = 80; i < 85; i++) {
        std::cout << "  stack[" << i << "] = 0x" << std::hex << cpu->stack[i] << std::dec << "\n";
    }
    std::cout << "\n";
}

// Execute the ROP chain
void execute_rop(CPU* cpu) {
    std::cout << "=== Executing ROP chain ===\n";
    
    // Function returns - first "return" starts the chain
    cpu->rip = cpu->stack[cpu->rsp];
    cpu->rsp++;
    
    print_cpu(cpu);
    
    // Execute gadgets based on RIP value
    while (cpu->rip != 0) {
        if (cpu->rip == 0x401234) {
            gadget_pop_rax(cpu);
        } else if (cpu->rip == 0x401567) {
            gadget_add_rax_rbx(cpu);
        } else if (cpu->rip == 0x401890) {
            gadget_mov_mem(cpu);
        }
        
        print_cpu(cpu);
    }
    
    std::cout << "ROP chain complete!\n\n";
}

// Show what the gadgets look like in assembly
void show_gadget_discovery() {
    std::cout << "=== How gadgets are found ===\n";
    std::cout << "Scanning binary for instruction sequences ending in RET:\n\n";
    
    std::cout << "0x401230: mov eax, ebx\n";
    std::cout << "0x401232: push ecx\n"; 
    std::cout << "0x401234: pop rax        <- Gadget 1 starts here\n";
    std::cout << "0x401235: ret            <- Ends with RET\n\n";
    
    std::cout << "0x401565: xor eax, eax\n";
    std::cout << "0x401567: add rax, rbx   <- Gadget 2\n";
    std::cout << "0x401569: ret\n\n";
    
    std::cout << "0x401888: push rdi\n";
    std::cout << "0x401890: mov [rbx], rax <- Gadget 3\n";
    std::cout << "0x401892: ret\n\n";
}

int main() {
    std::cout << "=== Simple ROP Gadget Demo ===\n\n";
    
    CPU cpu;
    init_cpu(&cpu);
    
    show_gadget_discovery();
    setup_rop_chain(&cpu);
    execute_rop(&cpu);
        
    return 0;
}