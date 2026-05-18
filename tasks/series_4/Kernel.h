#include <cstdint>
#include <cstddef>
#include <vector>

#ifndef MINI_JIT_KERNEL_H
#define MINI_JIT_KERNEL_H

namespace MiniJit
{
	class Kernel;
}

class MiniJit::Kernel
{
private:
	//! high-level code buffer
	std::vector<uint32_t> m_buffer;

	//! size of the kernel
	std::size_t m_size_alloc = 0;

	//! executable kernel
	void *m_kernel = nullptr;

	/**
	 * Allocates memory through POSIX mmap.
	 *
	 * @param num_bytes number of bytes.
	 **/
	void *alloc_mmap(std::size_t num_bytes) const;

	/**
	 * Release POSIX mmap allocated memory.
	 *
	 * @param num_bytes number of bytes.
	 * @param mem pointer to memory which is released.
	 **/
	void release_mmap(std::size_t num_bytes,
					  void *mem) const;

	/**
	 * Sets the given memory region executable.
	 *
	 * @param num_bytes number of bytes.
	 * @param mem point to memory.
	 **/
	void set_exec(std::size_t num_bytes,
				  void *mem) const;

	/**
	 * Release memory of the kernel if allocated.
	 **/
	void release_memory();

public:
	/**
	 * Constructor
	 **/
	Kernel() {};

	/**
	 * Destructor
	 **/
	~Kernel() noexcept;

	Kernel(Kernel const &) = delete;
	Kernel &operator=(Kernel const &) = delete;
	Kernel(Kernel &&) noexcept = delete;
	Kernel &operator=(Kernel &&) noexcept = delete;

	//! Represents a location in the instruction buffer
    using Label = std::size_t;

    /**
     * @brief Gets the current position in the code buffer.
     * Use this to mark jump targets (like .Lloop or .Lend)
     **/
    Label get_label() const;

    /**
     * @brief Calculates the instruction offset from the current end of the buffer to a target label.
     **/
    int32_t calc_offset(Label target) const;

    /**
     * @brief Overwrites an instruction at a specific index (used for backpatching forward branches).
     **/
    void patch_instruction(Label branch_idx, uint32_t new_inst);

	/**
	 * Adds an instruction to the code buffer.
	 *
	 * @param ins instruction which is added.
	 **/
	void add_instr(uint32_t ins);

	/**
     * @brief Adds a vector of instructions to the code buffer.
     * 
     * @param instrs vector of instructions.
     **/
    void add_instrs(const std::vector<uint32_t>& instrs);

	/**
	 * Gets the size of the code buffer.
	 *
	 * @return size of the code buffer in bytes.
	 **/
	std::size_t get_size() const;

	/**
	 * Sets the kernel based on the code buffer.
	 **/
	void set_kernel();

	/**
	 * Gets a pointer to the executable kernel.
	 **/
	void const *get_kernel() const;

	/**
	 * Writes the code buffer to the given file.
	 *
	 * @param path path to the file.
	 **/
	void write(char const *path) const;
};

#endif