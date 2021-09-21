# mp1.S - missile-command version
# Good luck, and have fun!
# Tab: 4
# Coding by: Yuhang Chen

.data

    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 		= 0
	X			= 4
	Y	  		= 8
	VX	  		= 12
	VY	  		= 16
	DEST_X  	= 20
	DEST_Y	  	= 24
	EXPLODED	= 28
	C	  		= 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:			.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:			.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

mp1_ioctl_status:	.long 0x0	# the status of mp1_ioctl

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX (Instead, make edx Callee saved register)

mp1_poke:
	pushl	%edx 
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	popl	%edx
	ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
#            %ecx: pointer to each missile_list struct

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%edi
	pushl	%esi

	movl	mp1_missile_list, %ecx # pointer to missile 
	movl	$0, %esi
	
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	# Task 1: update_missiles
	call	update_missiles
	# Task 2&3: Redraw the bases and redraw the crosshairs  
	movl	%eax, %esi
	call	redraw_bases
	call	redraw_crosshairs
	popl	%edx
	popl	%ecx
	popl	%eax

	# Judge the status of the game
	cmpl	$0, %esi
	je		tasklet_end
	# call mp1_notify_user to notify the user
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	call	mp1_notify_user
	popl	%edx
	popl	%ecx
	popl	%eax

tasklet_end:
	popl	%esi
	popl	%edi
	popl	%ebx
	leave
	ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
	# get the value of cmd in %ecx
	movl 	8(%esp), %ecx
	cmpl 	$4, %ecx
	jg 	 	mp1_ioctl_wrong
	cmpl 	$0, %ecx
	jl	 	mp1_ioctl_wrong
	jmp	 	*mp1_core_functions(,%ecx,4)

mp1_ioctl_wrong:
	movl	$-1, %ecx
	ret

# Jump table to get the core functions
mp1_core_functions:
	.long	mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame


# ----------------- Functions private to this file -------------------
update_missiles:
# Task 1: this function updates the missiles list
# Inputs   : %ecx: the pointer point to the current missiles list
# Outputs  : %eax: return value for func. mp1_notify_user
# Registers: Standard C calling convention

	cmpl	$0, %ecx # check whether the missiles list has reached the end (NULL value: $0)
	jne		GoOn_update_missiles
	movl	%esi, %eax
	ret

GoOn_update_missiles:
	# the first time to remove it from the screen
	call	get_position
	pushl	%ecx
	movb	$32, %cl
	call	mp1_poke
	popl	%ecx
	# check whether the missile is exploding
	cmpl	$0, EXPLODED(%ecx)
	jne		no_x_y_update

	# update the x and y of this missile
	movl	VX(%ecx), %edx
	addl	%edx, X(%ecx)
	movl	VY(%ecx), %edx
	addl	%edx, Y(%ecx)

no_x_y_update:
	# check whether it is out of screen
	cmpl	$0x4fffff, X(%ecx)
	jg		erase_missile

	cmpl	$0, X(%ecx)
	jl		erase_missile
	cmpl	$0x18ffff, Y(%ecx)
	jg		erase_missile
	cmpl	$0, Y(%ecx)
	jl		erase_missile
	jmp		second_case

erase_missile:
	call	mp1_missile_remove
	jmp		this_missile_done

second_case:	
	#first check if it is exploding
	cmpl	$0, EXPLODED(%ecx)
	jne		is_exploding
	movl	X(%ecx), %edx
	shrl	$16, %edx
	cmpl	%edx, DEST_X(%ecx)
	jne		third_case
	movl	Y(%ecx), %edx
	shrl	$16, %edx
	cmpl	%edx, DEST_Y(%ecx)
	jne		third_case

is_exploding:
	#save caller saved registers
	pushl	%eax
	pushl	%edx		
	#push the argument for missile_explode
	pushl	%ecx
	call	missile_explode
	#check if this function returns non_zero value
	cmpl	$0, %esi
	jne		is_exploding_continue
	movl	%eax, %esi
is_exploding_continue:
	popl	%ecx
	popl	%edx
	popl	%eax
	#get the position of the missile in %eax
	call	get_position
	subl	$1, EXPLODED(%ecx)
	jz		explode_stop
	pushl	%ecx
	movb	$EXPLOSION, %cl
	jmp		draw_explosion
explode_stop:
	call	mp1_missile_remove
	pushl	%ecx
	movb	$32,%cl
draw_explosion:	
	call	mp1_poke
	popl	%ecx
	jmp		this_missile_done		 
third_case:
	#this case just redraw the missile in new position
	call	get_position
	pushl	%ecx
	movb	C(%ecx), %cl
	call	mp1_poke
	popl	%ecx
this_missile_done:
	#move the %ecx points to the next missile
	cmpl	$0, %ecx
	jne		update_missiles_continue_second
	movl	%esi, %eax
	ret
update_missiles_continue_second:
	movl	NEXT(%ecx), %ecx
	jmp		update_missiles   

# this function redraws the three bases with particular status(alive or dead)
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
#			 %ecx: hold the left corner of the city to be drawn

redraw_bases:
		#draw the leftmost one
		movl	$18, %ecx
		#use %edx to status of each base
		movl	base_alive, %edx
		#get the last byte
		andl	$0x000000ff, %edx
		cmpl	$0, %edx
		je		draw_first_destroyed_base
		call	draw_base
		jmp		draw_second_base

draw_first_destroyed_base:
		call	draw_destroyed_base

draw_second_base:
		movl	$38, %ecx
		movl	base_alive, %edx
		
		andl	$0xff00, %edx
		cmpl	$0, %edx
		je		draw_second_destroyed_base
		call	draw_base
		jmp		draw_third_base

draw_second_destroyed_base:
		call	draw_destroyed_base

draw_third_base:
		movl	$58, %ecx
		movl	base_alive, %edx
		#get the second byte
		andl	$0x00ff0000, %edx
		cmpl	$0, %edx
		jne		draw_third_good_base
		call	draw_destroyed_base
		ret

draw_third_good_base:
		call	draw_base
		ret

# Task 3: Redraw the crossharis
#this function redraws the crosshair
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
redraw_crosshairs:
		#this function redraws the crosshair
		# position of (80*y+x)*2
		movl	crosshairs_y, %eax
		imull	$80, %eax
		addl	crosshairs_x, %eax
		addl	%eax, %eax
		#use "+" as crosshairs, whose ascii is 43 in decimal
		pushl	%ecx
		movb	$43, %cl
		call	mp1_poke
		popl	%ecx
		ret

# this function is called by mp1_ioctl and intialize the base_alive and corss_hairs
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
mp1_ioctl_startgame:
		#c calling convetion
		pushl	%ebp
		movl	%esp, %ebp
		pushl	%ebx
		pushl	%edi
		pushl	%esi

		movl	$0x00010101, base_alive
		movl	$12, crosshairs_y
		movl	$40, crosshairs_x
		
		popl	%esi
		pushl	%edi
		popl	%ebx
		leave     
		ret  

# this function copy a user space missile to kernel called by mp1_ioctl
# Inputs   : one argument, points to the missile in user space
# Outputs  : return 0 if copy succeeds and -1 if fails
# Registers: Standard C calling convention
#		  	 %eax: holds the pointer to the allocated memory in kernel
mp1_ioctl_addmissile:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	
	movl	8(%ebp), %ecx
	# allocate 33 bytes in kernel
	movl	$33, %edx
	pushl	%ecx
	pushl	%edx
	call	mp1_malloc
	popl	%edx
	popl	%ecx
	
	pushl	%edx
	pushl	%ecx
	pushl	%eax
	call	mp1_copy_from_user
	# check if the return value is 0 (fail)
	cmpl	$0, %eax
	jne		addmissile_fail
	popl	%eax
	popl	%ecx
	popl	%edx
	
	# then we make %edx->next = head, head = %edx
	movl	mp1_missile_list, %edx
	movl	%edx, NEXT(%eax)
	movl	%eax, mp1_missile_list

	movl	$0, %eax
	jmp		addmissile_end
addmissile_fail:
	popl	%eax
	popl	%ecx
	popl	%edx
	# free memory, %eax still holds the allocated memory
	pushl	%eax
	call	mp1_free
	popl	%eax
	#return -1
	movl	$-1, %eax

addmissile_end:
	popl	%esi
	popl	%edi
	popl	%ebx
	leave     
	ret   

# this function will move the x hair called by mp1_ioctl
# Inputs   : one argument, contains the move in x in low 16 bits and move in y in high 16 bits
# Outputs  : return 0 and never fails
# Registers: Standard C calling convention
mp1_ioctl_movexhairs:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	
	# position of (80*y+x)*2
	movl	crosshairs_y, %eax
	imull	$80, %eax
	addl	crosshairs_x, %eax
	addl	%eax, %eax

	
	movb	$32, %cl	# " ": ASCII 32 
	call	mp1_poke
	#then get the argument in %ecx
	movl	8(%ebp), %ecx
	cmpl	$0, %ecx
	je		movexhairs_end
	#then we want to save x in %eax and y in %ecx
	movl	%ecx, %eax
	shll	$16, %eax
	sarl	$16, %eax
	sarl	$16, %ecx
	#then add the change and check boundaries
	addl	%eax, crosshairs_x
	cmpl	$79, crosshairs_x
	jg		movexhairs_xTooBig
	cmpl	$0, crosshairs_x
	jl		movexhairs_xTooSmall
	jmp		movexhairs_checkY
movexhairs_xtooBig:
	movl	$79, crosshairs_x
	jmp		movexhairs_checkY
movexhairs_xtooSmall:
	movl	$0, crosshairs_x

movexhairs_checkY:
	addl	%ecx, crosshairs_y
	cmpl	$24, crosshairs_y
	jg		movexhairs_yTooBig
	cmpl	$0, crosshairs_y
	jl		movexhairs_yTooSmall
	jmp		movexhairs_redraw
movexhairs_yTooBig:
	movl	$24, crosshairs_y
	jmp		movexhairs_redraw
movexhairs_yTooSmall:
	movl	$0, crosshairs_y
movexhairs_redraw:
	movl	crosshairs_y, %eax
	imull	$80, %eax
	addl	crosshairs_x, %eax
	addl	%eax, %eax
	# "+" as crosshairs: ASCII 53
	movb	$43, %cl
	call	mp1_poke
movexhairs_end:
	#prepare to leave
	movl	$0, %eax
	
	popl	%esi
	popl	%edi
	popl	%ebx
	leave          
	ret

# this function is called by mp1_ioctl and get the status of the game
# Inputs   : one argument, points to user space that we need to move data to
# Outputs  : return 0 if copy succeeds and -1 if fails
# Registers: Standard C calling convention
mp1_ioctl_getstatus:
	pushl 	%ebp
	movl 	%esp,%ebp
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	# get the argument
	movl	8(%ebp),%ecx
	# initialize 0 to the status
	movl 	$0, mp1_ioctl_status			
	# copy score into the lower 16 bits
	movl	mp1_score, %edx				
	movl	%edx, mp1_ioctl_status

status_check_first:
	#check if first base alive and set bit 16
	movl	$0x000000ff, %eax			
	andl	base_alive, %eax
	jz		status_check_second
	addl	$0x00010000, mp1_ioctl_status

status_check_second:
	# check whether second base alive and set bit 17
	movl	$0x0000ff00, %eax			
	andl	base_alive, %eax
	jz		status_check_third
	addl	$0x20000, mp1_ioctl_status	

status_check_third:
	# check whether third base alive and set bit 18
	movl	$0x00ff0000, %eax			
	andl	base_alive, %eax
	jz		getstatus_end
	addl	$0x00040000, mp1_ioctl_status

getstatus_end:			
	pushl	$4
	pushl	$mp1_ioctl_status
	pushl	%ecx
	call	mp1_copy_to_user
	cmpl	$0, %eax					
	#clean up the stack
	popl	%edx							
	popl	%edx						
	popl	%edx										
	jz	status_out						
	movl	$-1, %eax					
status_out:	
	popl	%ebx
	popl	%edi
	popl	%esi
	leave
	ret

# this function is called by mp1_ioctl and clean the list and free all the memory
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
mp1_ioctl_endgame:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	# the function clean up the linked list
endgame_clear_list:
	cmpl	$0, mp1_missile_list
	je		endgame_end
	#%eax = head, head = head->next, free(%eax)
	movl	mp1_missile_list, %eax
	movl	NEXT(%eax), %edx
	movl	%edx, mp1_missile_list
	pushl	%eax
	call	mp1_free
	popl	%eax
	jmp		endgame_clear_list
endgame_end:
	#prepare to leave
	movl	$1, %eax
	
	popl	%esi
	popl	%edi
	popl	%ebx
	leave
	ret


#   ----------------- Helper Functions ---------------------
# 1) get_position, 2) mp1_missile_remove, 3) draw_base, 4) draw_destroyed_base
# with all callee saved registers 

# 1) get_position Function
# this function is get the position on the screen by calculation
# Inputs   : %ecx points to the missile
# Outputs  : return the offset to print to the screen
# Registers: Standard C calling convention
#			 %ecx: the pointer to missile 
get_position:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	
	
	pushl	%edx
	movl	Y(%ecx), %edx
	shrl	$16, %edx
	movl	%edx, %eax
	imull	$80, %eax
	movl	X(%ecx), %edx
	shrl 	$16, %edx
	addl	%edx, %eax
	addl	%eax, %eax
	popl	%edx
	
	popl	%esi
	popl	%edi
	popl	%ebx
	leave
	ret

# this function is used to remove the missile 
# Inputs   : %ecx points to the missile need to be removed
# Outputs  : none
# Registers: Standard C calling convention
# 			 %ecx : hold the missile need to be removed
#   	     %esi : trace the linked list
mp1_missile_remove:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	
	cmpl	$0, mp1_missile_list
	je		missile_remove_end
	movl	mp1_missile_list, %esi
	# check whether it is the first one
	cmpl	%esi, %ecx
	je		missile_remove_firstone

	# check if %esi -> next == %ecx

search_front_missile:
	cmpl	NEXT(%esi), %ecx
	je		find_front_missile
	cmpl	$0, NEXT(%esi)
	je		missile_remove_end
	movl	NEXT(%esi), %esi
	jmp		search_front_missile
find_front_missile:
	# copy the "next" of %ecx to the "next" of %esi
	movl	NEXT(%ecx),	%edx
	movl	%edx, NEXT(%esi)
	
	pushl	%eax
	pushl	%edx
	
	pushl	%ecx
	call	mp1_free
	popl	%ecx
	popl	%edx
	popl	%eax
	#make %ecx points to the missile in front of the removed one
	movl	%esi, %ecx
	jmp		missile_remove_end

missile_remove_firstone:
	movl	NEXT(%ecx), %ebx
	movl	%ebx, mp1_missile_list
	
	pushl	%eax
	pushl	%edx
	pushl	%ecx
	call	mp1_free
	popl	%ecx
	popl	%edx
	popl	%eax
	movl	mp1_missile_list, %ecx

missile_remove_end:
	popl	%esi
	popl	%edi
	popl	%ebx
	leave
	ret
	
# this function is draw the alive city on the screen 
# Inputs   : %ecx: the leftmost corner of the city
# Outputs  : none
# Registers: Standard C calling convention
#			 %esi: hold the base_pic address
#			 %ecx:  the leftcorner of city
draw_base:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	
	leal	base_pic, %esi
	
	movl	$24, %eax
	imull	$80, %eax
	addl	%ecx, %eax
	addl	%eax, %eax
	# draw the first char
	movb	0(%esi), %cl
	call	mp1_poke
	
	# draw the rest char
	# add one char to %eax each time, which is 2 bytes
	addl	$2, %eax
	movb	1(%esi), %cl
	call	mp1_poke
	addl	$2, %eax
	movb	2(%esi), %cl
	call	mp1_poke
	addl	$2, %eax
	movb	3(%esi), %cl
	call	mp1_poke
	addl	$2, %eax
	movb	4(%esi), %cl
	call	mp1_poke
	
	popl	%esi
	popl	%edi
	popl	%ebx
	leave
	ret		

	
# this function is to draw the dead city on the screen 
# Inputs   : %ecx holds the leftmost corner of the city
# Outputs  : none
# Registers: Standard C calling convention	
# 			 %esi: hold the address of dead_base_picture	
draw_destroyed_base:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	
	leal	dead_base_pic, %esi
	
	movl	$24, %eax
	imull	$80, %eax
	addl	%ecx, %eax
	addl	%eax, %eax
	# the first char on the screen
	movb	0(%esi), %cl
	call	mp1_poke

	# the rest part of char
	addl	$2, %eax
	movb	1(%esi), %cl
	call	mp1_poke
	addl	$2, %eax
	movb	2(%esi), %cl
	call	mp1_poke
	addl	$2, %eax
	movb	3(%esi), %cl
	call	mp1_poke
	addl	$2, %eax
	movb	4(%esi), %cl
	call	mp1_poke

	popl	%esi
	popl	%edi
	popl	%ebx
	leave
	ret