#pragma once

# define INTERNAL_SYSCALL_NCS(number, nr, args...) \
	internal_syscall##nr (number, args)

# define internal_syscall0(number, dummy...)			\
({ 									\
	long int _sys_result;						\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0");				\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "=r" (__a0)							\
	: "r" (__a7)							\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall1(number, arg0)				\
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7)							\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall2(number, arg0, arg1)	    		\
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
	long int _arg1 = (long int) (arg1);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	register long int __a1 asm ("a1") = _arg1;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7), "r" (__a1)					\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall3(number, arg0, arg1, arg2)      		\
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
	long int _arg1 = (long int) (arg1);				\
	long int _arg2 = (long int) (arg2);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	register long int __a1 asm ("a1") = _arg1;			\
	register long int __a2 asm ("a2") = _arg2;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7), "r" (__a1), "r" (__a2)				\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall4(number, arg0, arg1, arg2, arg3)	  \
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
	long int _arg1 = (long int) (arg1);				\
	long int _arg2 = (long int) (arg2);				\
	long int _arg3 = (long int) (arg3);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	register long int __a1 asm ("a1") = _arg1;			\
	register long int __a2 asm ("a2") = _arg2;			\
	register long int __a3 asm ("a3") = _arg3;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7), "r" (__a1), "r" (__a2), "r" (__a3)		\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall5(number, arg0, arg1, arg2, arg3, arg4)   \
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
	long int _arg1 = (long int) (arg1);				\
	long int _arg2 = (long int) (arg2);				\
	long int _arg3 = (long int) (arg3);				\
	long int _arg4 = (long int) (arg4);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	register long int __a1 asm ("a1") = _arg1;			\
	register long int __a2 asm ("a2") = _arg2;			\
	register long int __a3 asm ("a3") = _arg3;			\
	register long int __a4 asm ("a4") = _arg4;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7), "r"(__a1), "r"(__a2), "r"(__a3), "r" (__a4)	\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall6(number, arg0, arg1, arg2, arg3, arg4, arg5) \
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
	long int _arg1 = (long int) (arg1);				\
	long int _arg2 = (long int) (arg2);				\
	long int _arg3 = (long int) (arg3);				\
	long int _arg4 = (long int) (arg4);				\
	long int _arg5 = (long int) (arg5);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	register long int __a1 asm ("a1") = _arg1;			\
	register long int __a2 asm ("a2") = _arg2;			\
	register long int __a3 asm ("a3") = _arg3;			\
	register long int __a4 asm ("a4") = _arg4;			\
	register long int __a5 asm ("a5") = _arg5;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7), "r" (__a1), "r" (__a2), "r" (__a3),		\
	  "r" (__a4), "r" (__a5)					\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define internal_syscall7(number, arg0, arg1, arg2, arg3, arg4, arg5, arg6) \
({ 									\
	long int _sys_result;						\
	long int _arg0 = (long int) (arg0);				\
	long int _arg1 = (long int) (arg1);				\
	long int _arg2 = (long int) (arg2);				\
	long int _arg3 = (long int) (arg3);				\
	long int _arg4 = (long int) (arg4);				\
	long int _arg5 = (long int) (arg5);				\
	long int _arg6 = (long int) (arg6);				\
									\
	{								\
	register long int __a7 asm ("a7") = number;			\
	register long int __a0 asm ("a0") = _arg0;			\
	register long int __a1 asm ("a1") = _arg1;			\
	register long int __a2 asm ("a2") = _arg2;			\
	register long int __a3 asm ("a3") = _arg3;			\
	register long int __a4 asm ("a4") = _arg4;			\
	register long int __a5 asm ("a5") = _arg5;			\
	register long int __a6 asm ("a6") = _arg6;			\
	__asm__ volatile ( 						\
	"ecall\n\t" 							\
	: "+r" (__a0)							\
	: "r" (__a7), "r" (__a1), "r" (__a2), "r" (__a3),		\
	  "r" (__a4), "r" (__a5), "r" (__a6)				\
	: __SYSCALL_CLOBBERS); 						\
	_sys_result = __a0;						\
	}								\
	_sys_result;							\
})

# define __SYSCALL_CLOBBERS "memory"
