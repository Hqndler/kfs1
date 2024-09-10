CC = i386-elf-gcc
AS = nasm
CFLAGS = -fno-builtin -fno-exceptions -fno-stack-protector -nostdlib -nodefaultlibs -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -MMD
NAME = ourKernel
NAME_BIN = $(NAME).bin
NAME_ISO = $(NAME).iso

SRC_DIR = srcs/
OBJ_DIR = obj/
DEP_DIR = deps/
ISO_DIR = iso_dir/

FILES_C = kernel
FILES_ASM = boot io

OBJS = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(FILES_C)))
OBJS += $(addprefix $(OBJ_DIR), $(addsuffix .o, $(FILES_ASM)))

DEPS = $(patsubst $(SRC_DIR)%.c, $(DEP_DIR)%.d, $(notdir $(FILES)))

INCLUDE	= -I ./include

vpath %.c $(SRC_DIR)
vpath %.s $(SRC_DIR)

all: $(NAME_BIN)

$(NAME_BIN): $(OBJS)
	@$(CC) -T linker.ld -o $(NAME_BIN) $(CFLAGS) $(OBJS) -lgcc
	@echo Compilation done !
	@if grub-file --is-x86-multiboot ourKernel.bin; then\
        echo "Multiboot confirmed !";\
	else\
        echo "Not multibootable !";\
    fi

$(OBJ_DIR)%.o : %.c
	@ mkdir -p $(OBJ_DIR) $(DEP_DIR)
	@ $(CC) $(CFLAGS) $(INCLUDE) -MF $(patsubst $(OBJ_DIR)%.o, $(DEP_DIR)%.d, $@) -c $< -o $@

$(OBJ_DIR)%.o : %.s
	@ mkdir -p $(OBJ_DIR) $(DEP_DIR)
	@ $(AS) -f elf32 $< -o $@

run_kernel:
	@qemu-system-i386 -kernel $(NAME_BIN)

check_format:
	@readelf $(NAME_BIN) -h

iso:
	@mkdir -p $(ISO_DIR)boot/grub
	@cp $(NAME_BIN) $(ISO_DIR)boot/$(NAME_BIN)
	@cp grub.cfg $(ISO_DIR)boot/grub/grub.cfg
	@grub-mkrescue -o $(NAME_ISO) $(ISO_DIR)

run_iso:
	@qemu-system-i386 -cdrom $(NAME_ISO)

clean:
	@rm -rf $(OBJ_DIR)
	@rm -rf $(DEP_DIR)
	@rm -rf $(ISO_DIR)

fclean: clean
	@rm -rf $(NAME_BIN)
	@rm -rf $(NAME_ISO)

re: fclean all

.PHONY: all clean fclean re run_kernel iso run_iso