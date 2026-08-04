//
// Created by chris on 8/10/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_LINUX_H
#define MLA_GLOBAL_PLATFORM_LINUX_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>

#ifndef mla_use_stacktraces
#define mla_use_stacktraces 1
#endif

#if mla_use_stacktraces == 1
#include <execinfo.h>
#endif
#include <cxxabi.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdlib.h>

#include <string.h>
#include <elf.h>

#if mla_use_fast_float == 1

// Using fast-float library for optimized float parsing
// https://github.com/fastfloat/fast_float
// Is found about 40% faster than standard library
// the code size is growing by about 15KB

#include "../generic/mla_global_platform_fast_float.h"

#define mla_platform_strtod mla_private_fast_float_strtod
#define mla_platform_strtoll mla_private_fast_float_strtoll
#define mla_platform_strtoull mla_private_fast_float_strtoull

#else

#define mla_platform_strtod mla_private_generic_strtod
#define mla_platform_strtoll mla_private_generic_strtoll
#define mla_platform_strtoull mla_private_generic_strtoull

#endif


void mla_private_linux_sleep(mla_uint32_t milliseconds) {

    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

mla_uint64_t mla_private_linux_system_time_ms() {

    struct timespec ts;
    // Uses a faster, less precise clock source usually sufficient for millisecond-level timing
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (mla_s_cast<mla_uint64_t>(ts.tv_sec) * 1000) + (mla_s_cast<mla_uint64_t>(ts.tv_nsec) / 1000000);

}

/**
 * @brief Reads available raw input bytes from standard input without blocking.
 *
 * Configures the terminal interface into non-canonical, non-echoing mode, sets
 * STDIN_FILENO to non-blocking, and reads available bytes directly via POSIX read()
 * to prevent stdio ferror/EOF lockups on non-blocking reads.
 *
 * @param buffer Output buffer to receive raw input characters.
 * @param size Maximum size of the output buffer (including null terminator).
 * @return Number of characters read into buffer.
 */
mla_size_t mla_private_linux_std_read(mla_char_t* buffer, mla_size_t size) {
    if (buffer == nullptr || size <= 1) {
        return 0;
    }

    struct termios oldt;
    struct termios newt;
    int oldf;

    // Save terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo. This is a raw byte pump: it does NOT echo
    // or translate anything. Line editing (echo, cursor movement, history,
    // autocomplete) is handled by the CLI line editor in mla_cli_app.cpp, which
    // needs to see every raw byte - including escape sequences for arrow keys.
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Make stdin non-blocking
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    // Clear any stdio error or EOF flags on stdin stream before reading
    clearerr(stdin);

    // Read available bytes directly from file descriptor 0 (STDIN_FILENO) using
    // POSIX read(). This avoids glibc stdio internal FILE* error flag sticky states.
    ssize_t bytes_read = read(STDIN_FILENO, buffer, size - 1);
    mla_size_t count = 0;

    if (bytes_read > 0) {
        count = mla_s_cast<mla_size_t>(bytes_read);
    }

    buffer[count] = '\0';

    // Clear stdio error state again and restore terminal settings and flags
    clearerr(stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    return count;
}

bool mla_private_linux_is_internal_exception_frame(const char* name) {
    if (name == nullptr) {
        return false;
    }
    if (mla_strstr(name, "mla_private_linux_get_stack_trace") != nullptr ||
        mla_strstr(name, "mla_private_generic_on_malloc_failure") != nullptr ||
        mla_strstr(name, "mla_private_linux_signal_handler") != nullptr ||
        mla_strstr(name, "__restore_rt") != nullptr ||
        mla_strstr(name, "backtrace") != nullptr) {
        return true;
    }
    return false;
}

inline bool mla_private_linux_resolve_elf_symbol(const char* binary_path, uintptr_t relative_addr, char* out_func, size_t func_size) {
    if (out_func == nullptr || func_size == 0) {
        return false;
    }
    out_func[0] = '\0';

    if (binary_path == nullptr || binary_path[0] == '\0') {
        binary_path = "/proc/self/exe";
    }

    int fd = open(binary_path, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    Elf64_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != mla_s_cast<ssize_t>(sizeof(ehdr))) {
        close(fd);
        return false;
    }

    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        close(fd);
        return false;
    }

    if (ehdr.e_shoff == 0 || ehdr.e_shentsize != sizeof(Elf64_Shdr)) {
        close(fd);
        return false;
    }

    const size_t shdr_size = ehdr.e_shnum * sizeof(Elf64_Shdr);
    Elf64_Shdr* shdrs = mla_r_cast<Elf64_Shdr*>(malloc(shdr_size));
    if (shdrs == nullptr) {
        close(fd);
        return false;
    }

    if (lseek(fd, mla_s_cast<off_t>(ehdr.e_shoff), SEEK_SET) < 0 ||
        read(fd, shdrs, shdr_size) != mla_s_cast<ssize_t>(shdr_size)) {
        free(shdrs);
        close(fd);
        return false;
    }

    const Elf64_Shdr* symtab_sh = nullptr;
    const Elf64_Shdr* strtab_sh = nullptr;

    for (uint16_t i = 0; i < ehdr.e_shnum; ++i) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab_sh = &shdrs[i];
            if (symtab_sh->sh_link < ehdr.e_shnum) {
                strtab_sh = &shdrs[symtab_sh->sh_link];
            }
            break;
        }
    }

    if (symtab_sh == nullptr || strtab_sh == nullptr || symtab_sh->sh_entsize != sizeof(Elf64_Sym)) {
        free(shdrs);
        close(fd);
        return false;
    }

    Elf64_Sym* syms = mla_r_cast<Elf64_Sym*>(malloc(symtab_sh->sh_size));
    if (syms == nullptr) {
        free(shdrs);
        close(fd);
        return false;
    }

    if (lseek(fd, mla_s_cast<off_t>(symtab_sh->sh_offset), SEEK_SET) < 0 ||
        read(fd, syms, symtab_sh->sh_size) != mla_s_cast<ssize_t>(symtab_sh->sh_size)) {
        free(syms);
        free(shdrs);
        close(fd);
        return false;
    }

    char* strtab = mla_r_cast<char*>(malloc(strtab_sh->sh_size));
    if (strtab == nullptr) {
        free(syms);
        free(shdrs);
        close(fd);
        return false;
    }

    if (lseek(fd, mla_s_cast<off_t>(strtab_sh->sh_offset), SEEK_SET) < 0 ||
        read(fd, strtab, strtab_sh->sh_size) != mla_s_cast<ssize_t>(strtab_sh->sh_size)) {
        free(strtab);
        free(syms);
        free(shdrs);
        close(fd);
        return false;
    }

    const size_t num_syms = symtab_sh->sh_size / sizeof(Elf64_Sym);
    const char* matched_name = nullptr;

    for (size_t i = 0; i < num_syms; ++i) {
        const uint8_t type = ELF64_ST_TYPE(syms[i].st_info);
        if (type == STT_FUNC || type == STT_NOTYPE) {
            if (syms[i].st_value != 0 && syms[i].st_name < strtab_sh->sh_size) {
                const uintptr_t start = mla_s_cast<uintptr_t>(syms[i].st_value);
                const uintptr_t end = start + (syms[i].st_size > 0 ? mla_s_cast<uintptr_t>(syms[i].st_size) : 1);
                if (relative_addr >= start && relative_addr < end) {
                    matched_name = strtab + syms[i].st_name;
                    break;
                }
            }
        }
    }

    bool found = false;
    if (matched_name != nullptr && matched_name[0] != '\0') {
        int status = -1;
        char demangled_buf[1024];
        size_t demangled_len = sizeof(demangled_buf);
        char* demangled = abi::__cxa_demangle(matched_name, demangled_buf, &demangled_len, &status);
        if (status == 0 && demangled != nullptr) {
            snprintf(out_func, func_size, "%s", demangled);
        } else {
            snprintf(out_func, func_size, "%s", matched_name);
        }
        found = true;
    }

    free(strtab);
    free(syms);
    free(shdrs);
    close(fd);
    return found;
}

mla_size_t mla_private_linux_get_stack_trace(mla_char_t* buffer, mla_size_t buffer_size) {

    if (buffer == nullptr || buffer_size == 0) {
        return 0;
    }

    buffer[0] = '\0';

#if mla_use_stacktraces == 1
    constexpr int max_frames = 64;
    void* frames[max_frames];
    const int frame_count = backtrace(frames, max_frames);
    if (frame_count <= 0) {
        return 0;
    }

    bool skipping_internal_frames = true;
    unsigned int user_frame_index = 0;
    mla_size_t written = 0;

    for (int frame_index = 0; frame_index < frame_count && written < buffer_size - 1; ++frame_index) {
        void* addr = frames[frame_index];
        Dl_info info;
        const char* symbol_name = nullptr;
        char demangled_buf[1024];
        char elf_func_buf[512] = {0};
        char line[1024];
        int line_length = 0;
        unsigned long long offset = 0;

        if (dladdr(addr, &info) != 0) {
            if (info.dli_sname != nullptr) {
                symbol_name = info.dli_sname;
                offset = mla_s_cast<unsigned long long>(mla_r_cast<uintptr_t>(addr) - mla_r_cast<uintptr_t>(info.dli_saddr));

                int status = -1;
                char* demangled = abi::__cxa_demangle(symbol_name, demangled_buf, nullptr, &status);
                if (status == 0 && demangled != nullptr) {
                    symbol_name = demangled;
                }
            } else if (info.dli_fbase != nullptr) {
                uintptr_t rel_addr = mla_r_cast<uintptr_t>(addr) - mla_r_cast<uintptr_t>(info.dli_fbase);
                const char* target_path = (info.dli_fname != nullptr && info.dli_fname[0] != '\0') ? info.dli_fname : "/proc/self/exe";
                if (mla_private_linux_resolve_elf_symbol(target_path, rel_addr, elf_func_buf, sizeof(elf_func_buf))) {
                    symbol_name = elf_func_buf;
                    offset = mla_s_cast<unsigned long long>(rel_addr);
                }
            }
        }

        if (skipping_internal_frames) {
            if (symbol_name != nullptr && mla_private_linux_is_internal_exception_frame(symbol_name)) {
                continue;
            }
            skipping_internal_frames = false;
        }

        if (symbol_name != nullptr) {
            line_length = snprintf(
                line,
                sizeof(line),
                "#%u %s + 0x%llx\n",
                user_frame_index,
                symbol_name,
                offset);
        } else {
            line_length = snprintf(
                line,
                sizeof(line),
                "#%u 0x%llx\n",
                user_frame_index,
                mla_s_cast<unsigned long long>(mla_r_cast<uintptr_t>(addr)));
        }

        if (line_length <= 0) {
            continue;
        }

        mla_size_t copy_length = mla_s_cast<mla_size_t>(line_length);
        const mla_size_t remaining = buffer_size - 1 - written;
        if (copy_length > remaining) {
            copy_length = remaining;
        }

        mla_private_generic_memcpy(buffer + written, line, copy_length);
        written += copy_length;
        buffer[written] = '\0';
        user_frame_index++;
    }

    return written;
#else
    return 0;
#endif
}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access ={
    mla_private_generic_memcpy,
        mla_private_generic_memset,
        mla_private_generic_memcmp,
        mla_private_generic_memmove,
        mla_private_generic_strlen,
        mla_private_generic_strstr,
        mla_private_generic_malloc,
        mla_private_generic_free,
        mla_private_generic_on_malloc_failure,
        mla_private_generic_print,
        mla_private_linux_std_read,
        mla_platform_strtod,
        mla_platform_strtoll,
        mla_platform_strtoull,
    mla_private_linux_sleep,
    mla_private_linux_system_time_ms,
    mla_private_linux_get_stack_trace
};

// NOLINTBEGIN(bugprone-signal-handler)
extern "C" void mla_private_linux_signal_handler(int sig, siginfo_t* info, void* ucontext) {
    (void)ucontext;

    const char* sig_name = nullptr;
    switch (sig) {
        case SIGSEGV: sig_name = "SIGSEGV (Segmentation Fault)"; break;
        case SIGABRT: sig_name = "SIGABRT (Abort)"; break;
        case SIGFPE: sig_name = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL: sig_name = "SIGILL (Illegal Instruction)"; break;
        case SIGBUS: sig_name = "SIGBUS (Bus Error)"; break;
        default: sig_name = "Unknown Signal"; break;
    }

    const void* fault_addr = (info != nullptr) ? info->si_addr : nullptr;

    fprintf(stderr, "\n=================================================================\n");
    fprintf(stderr, "[MLA CRASH HANDLER] Signal Caught!\n");
    fprintf(stderr, "Signal: %d (%s)\n", sig, sig_name);
    fprintf(stderr, "Fault Address: 0x%p\n", fault_addr);
    fprintf(stderr, "=================================================================\n");
    fprintf(stderr, "Stack Trace:\n");

    mla_char_t stack_buf[4096];
    mla_private_linux_get_stack_trace(stack_buf, sizeof(stack_buf));
    if (stack_buf[0] != '\0') {
        fprintf(stderr, "%s", stack_buf);
    } else {
        fprintf(stderr, "<stack trace unavailable>\n");
    }
    fprintf(stderr, "=================================================================\n");
    fflush(stderr);

    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig, &sa, nullptr);
    raise(sig);
}

void mla_boot_os_application() {
    struct sigaction sa;
    sa.sa_sigaction = mla_private_linux_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);

    // Finish boot
    mla_lifecycle_fire_boot_events();
}
// NOLINTEND(bugprone-signal-handler)

#endif
