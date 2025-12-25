#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define EI_NIDENT 16

// ELF 64 位元檔案表頭（ELF Header）
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    // ELF 魔數與基本屬性（位元數、Endian、ABI 等）

    uint16_t e_type;
    // 檔案類型（ET_EXEC, ET_DYN, ET_REL...）

    uint16_t e_machine;
    // 目標機器架構（如 EM_X86_64）

    uint32_t e_version;
    // ELF 格式版本（通常為 EV_CURRENT）

    uint64_t e_entry;
    // 程式進入點的虛擬位址（第一條指令位置，無則為 0）

    uint64_t e_phoff;
    // Program Header Table 在檔案中的位移（位元組為單位，無則為 0）

    uint64_t e_shoff;
    // Section Header Table 在檔案中的位移（位元組為單位，無則為 0）

    uint32_t e_flags;
    // 與處理器相關的旗標（架構特定）

    uint16_t e_ehsize;
    // ELF Header 本身的大小（位元組）

    uint16_t e_phentsize;
    // 單一 Program Header 的大小（位元組）

    uint16_t e_phnum;
    // Program Header 的數量

    uint16_t e_shentsize;
    // 單一 Section Header 的大小（位元組）

    uint16_t e_shnum;
    // Section Header 的數量

    uint16_t e_shstrndx;
    // Section 名稱字串表（.shstrtab）在 Section Header Table 中的索引
} Elf64_Ehdr;



// ELF 64 位元 Program Header（描述如何將檔案映射到記憶體）
typedef struct {
    uint32_t p_type;
    // Segment 類型（PT_LOAD, PT_DYNAMIC, PT_INTERP...）

    uint32_t p_flags;
    // Segment 權限旗標（PF_R, PF_W, PF_X）

    uint64_t p_offset;
    // Segment 在檔案中的起始位移（byte offset）

    uint64_t p_vaddr;
    // Segment 映射到記憶體後的虛擬位址起點

    uint64_t p_paddr;
    // 實體位址（多數系統未使用，通常忽略）

    uint64_t p_filesz;
    // Segment 在檔案中所佔的大小（位元組）

    uint64_t p_memsz;
    // Segment 在記憶體中所需的大小（位元組，可能大於 p_filesz，如 .bss）

    uint64_t p_align;
    // Segment 在檔案與記憶體中的對齊需求；
    // 若 p_align > 1，必須為 2 的冪，
    // 且 p_vaddr % p_align == p_offset % p_align（通常為頁面大小）
} Elf64_Phdr;



//exit(42) system call
unsigned char exit_code[]={
    // mov rax, 60 (syscall number for exit)
    0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,
    // mov rdi, 42 (exit status)
    0x48, 0xc7, 0xc7, 0x2a, 0x00, 0x00, 0x00,
    // syscall
    0x0f, 0x05   
};

//write "Hello, ELF World!\n" and then exit
unsigned char hello_code[] = {
    // write(1, message, length)
    0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, // mov rax, 1 (sys_write)
    0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00, // mov rdi, 1 (stdout)
    0x48, 0x8d, 0x35, 0x19, 0x00, 0x00, 0x00, // lea rsi, [rip + 0x19] (message address)
    0x48, 0xc7, 0xc2, 0x12, 0x00, 0x00, 0x00, // mov rdx, 18 (message length)
    0x0f, 0x05,                               // syscall
    
    // exit(0)
    0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 60 (sys_exit)
    0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00, // mov rdi, 0 (status)
    0x0f, 0x05,                               // syscall
    
    // message: "Hello, ELF World!\n"
    'H', 'e', 'l', 'l', 'o', ',', ' ', 'E', 'L', 'F', ' ', 
    'W', 'o', 'r', 'l', 'd', '!', '\n'
};

//build ELF file header
void create_elf_header(Elf64_Ehdr *header,uint64_t e_entry_point){
    //ELF Identification  number
    unsigned char ident[16]={
        0x7f,'E','L','F', //magic number
        2, //64-bit
        1, //Little Endian
        1, //ELF version
        0, //OS ABI (System v)
        0 //ABI version
    };

    memset(header->e_ident,0,16);
    memcpy(header->e_ident,ident,9);

    header->e_type=2; //ET_EXEC
    header->e_machine=0x3e; //EM_X86_64
    header->e_version=1;
    header->e_entry=e_entry_point; //entry of program
    header->e_phoff=sizeof(Elf64_Ehdr); //program header table after the ELF header
    header->e_shoff=0; //no section header table
    header->e_flags=0;
    header->e_ehsize=sizeof(Elf64_Ehdr);
    header->e_phentsize=sizeof(Elf64_Phdr);
    header->e_phnum=1; //one program header
    header->e_shentsize=0;
    header->e_shnum = 0;
    header->e_shstrndx = 0;

}

//build program header
void create_program_header(Elf64_Phdr *phdr, uint64_t offset,uint64_t vaddr,uint64_t size){
    phdr->p_type=1; //PT_LOAD
    phdr->p_flags=5; //PF_R | PF_X
    phdr->p_offset=offset; //offset of  the file
    phdr->p_vaddr=vaddr; //virtual address
    phdr->p_paddr=0; //physical address
    phdr->p_filesz=size; //size of the file
    phdr->p_memsz=size; //size of the memory
    phdr->p_align=0x1000; //align to page size (4096 bytes)
    // Ensure alignment: p_vaddr % p_align == p_offset % p_align
    // This is already satisfied if both are aligned to page boundary
}

//make simple ELF file
void generate_elf_file(const char *filename,int type){
    FILE *fp=fopen(filename,"wb");
    if (!fp){
        perror("Failed to open file");
        return;
    }

    //choose code
    unsigned char *code;
    size_t code_size;
    uint64_t entry_point;
    uint64_t base_vaddr = 0x400000; // Base virtual address (page-aligned)

    if (type==0){
        code=exit_code;
        code_size=sizeof(exit_code);
        entry_point=base_vaddr + 0x78; // Entry point offset from base
    }else {
        code=hello_code;
        code_size=sizeof(hello_code);
        entry_point=base_vaddr + 0x78; // Entry point offset from base
    }

    //calculate offset to size
    uint64_t header_size=sizeof(Elf64_Ehdr)+sizeof(Elf64_Phdr);
    uint64_t code_offset=header_size;
    uint64_t segment_vaddr=base_vaddr; // Segment starts at page-aligned address
    uint64_t segment_size=code_offset+code_size; // Total segment size

    //build ELF header
    Elf64_Ehdr header;
    create_elf_header(&header,entry_point);

    //build program header - segment starts at file offset 0, maps to base_vaddr
    // This ensures p_vaddr % p_align == p_offset % p_align (both are 0)
    Elf64_Phdr phdr;
    create_program_header(&phdr,0,segment_vaddr,segment_size);

    //write to header file
    fwrite(&header,sizeof(Elf64_Ehdr),1,fp);

    //write to program header file
    fwrite(&phdr,sizeof(Elf64_Phdr),1,fp);

    //padding to program starting position (if need to align)
    if (ftell(fp) < code_offset) {
        uint64_t padding_size=code_offset-ftell(fp);
        unsigned char *padding=calloc(padding_size,1);
        fwrite(padding,1,padding_size,fp);
        free(padding);
    }

    //write to code file
    fwrite(code,1,code_size,fp);

    fclose(fp);

    
    //setting executable permission
    chmod(filename,0755);

    printf("Generated ELF file: %s\n", filename);
    printf("Code size: %lu bytes\n", code_size);
    printf("Entry point: 0x%lx\n", entry_point);
}

int main(){


    generate_elf_file("exit.elf",0);

    generate_elf_file("hello.elf",1);



    return 0;
}
