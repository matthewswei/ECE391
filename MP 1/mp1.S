
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64

# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score



# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

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
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
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

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    call update_missiles
    call draw_cities
	call draw_crosshair
    popl %edi
    popl %esi
    popl %ebx
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
	movl 8(%esp), %eax
	cmpl $0, %eax
	jl invalid
	cmpl $4, %eax
	jg invalid
	jmp *ioctl_functions(, %eax, 4)
	invalid:
		movl $-1, %eax
		ret
	ioctl_functions:
		.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame
	ret

# ----------------- Functions private to this file -------------------

# update_missiles
# Goes through list of missiles and updates their coordinates, removes them, or changes their explosion values
update_missiles:
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	movl mp1_missile_list, %esi
	next_missile_loop:
		cmpl $0, %esi   				# Checks for empty node
		je finish  
		movl X(%esi), %eax 				# Prints missile to screen
		sarl $16, %eax
		imull $2, %eax
		movl Y(%esi), %ebx
		sarl $16, %ebx
		imull $160, %ebx
		addl %ebx, %eax   
		movb $32, %cl
		pushl %esi
		call mp1_poke
		popl %esi
		cmpl $0, EXPLODED(%esi)			# Checks if missile is exploding
		je update_coordinates
		jmp exploded
	update_coordinates:
		movl VX(%esi), %eax				# Checks if at border of screen
		addl X(%esi), %eax
		sarl $16, %eax
		cmpl $0, %eax
		jl mp1_missile_remove
		sarl $16, %eax
		cmpl $79, %eax
		jg mp1_missile_remove
		movl VY(%esi), %eax
		addl Y(%esi), %eax
		sarl $16, %eax
		cmpl $0, %eax 
		jl mp1_missile_remove
		sarl $16, %eax
		cmpl $24, %eax
		jg mp1_missile_remove	
		movl VX(%esi), %eax				# Updates the missile's position
		addl %eax, X(%esi)	
		movl VY(%esi), %eax
		addl %eax, Y(%esi)
		movl X(%esi), %eax				# Checks if missile reaches its destination
		sarl $16, %eax
		movl DEST_X(%esi), %ebx
		cmpl %eax, %ebx
		jne draw_missile
		movl Y(%esi), %eax
		sarl $16, %eax
		movl DEST_Y(%esi), %ebx
		cmpl %eax, %ebx
		je exploded		
		jmp draw_missile	                                                        
	exploded:
		movl X(%esi), %eax				# Gets missile's screen position and prints out explosion
		sarl $16, %eax
		imull $2, %eax
		movl Y(%esi), %ebx
		sarl $16, %ebx
		imull $160, %ebx
		addl %ebx, %eax 
		movb $EXPLOSION, %cl    
		pushl %esi
		call mp1_poke
		popl %esi
		pushl %esi 
		call missile_explode
		popl %esi
		cmpl $0, %eax					# Checks if any surrounding objects are caught in explosion
		je no_notify_user
		call mp1_notify_user
	no_notify_user:
		decl EXPLODED(%esi)     		# Decrease explosion value
		cmpl $0, EXPLODED(%esi)
		je remove_exploded
		jmp next_missile
	remove_exploded:					# Removes exploded object and missile from list
		movl X(%esi), %eax
		sarl $16, %eax
		imull $2, %eax
		movl Y(%esi), %ebx
		sarl $16, %ebx
		imull $160, %ebx
		addl %ebx, %eax 
		movb $32, %cl 
		pushl %esi
		call mp1_poke
		popl %esi
		call mp1_missile_remove
		jmp next_missile_loop
	draw_missile:   					# Prints missile to screen
		movl X(%esi), %eax                                
		sarl $16, %eax
		imull $2, %eax, %eax
		movl Y(%esi), %ebx                                 
		sarl $16, %ebx
		imull $160, %ebx
		addl %ebx, %eax 
		movb C(%esi), %cl 
		call mp1_poke   
		jmp next_missile
	next_missile:						# Moves on to next missile
		movl NEXT(%esi), %esi
		jmp next_missile_loop
	finish:      
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# mp1_missile_remove
# Sub-routine to remove missiles from update_missiles
mp1_missile_remove:
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %edi
		movl mp1_missile_list, %ecx
		cmpl %ecx, %esi
		je change_head
	remove_loop_start: 					# Find spot in list where node is
		cmpl NEXT(%ecx), %esi
		je change_node
		movl NEXT(%ecx), %ebx
		movl %ebx, %ecx
		jmp	remove_loop_start
	change_node:						# Remove node from list
		movl NEXT(%esi), %ebx
		movl %ebx, NEXT(%ecx)
		pushl %ecx
		pushl %esi
		call mp1_free
		popl %esi
		popl %ecx
		movl NEXT(%ecx), %esi
		jmp	remove_end
	change_head:						# Remove head of list
		movl NEXT(%ecx), %ebx
		movl %ebx, mp1_missile_list
		pushl %esi
		call mp1_free
		popl %esi
		movl mp1_missile_list, %esi
	remove_end:
		popl %edi
		popl %ebx
		leave
		ret

# draw_cities
# Draws the three bases onto the screen (alive and dead)
draw_cities:
	pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
	movl base_alive, %ebx
	movl $3876, %eax					# Puts initial starting position of base
	movl $3, %esi						# Counter for how many bases
	cities_start:						# Figures out if base is alive or dead
		movl $5, %edi
		movl %ebx, %edx
		andl $1, %edx
		cmpl $0, %edx
		je dead_city
		movl $base_pic, %edx
	alive_city:							# Prints out alive base
		movb (%edx), %cl
		pushl %edx
		call mp1_poke
		popl %edx
		decl %edi
		addl $2, %eax
		addl $1, %edx
		cmpl $0, %edi
		je next_city
		jmp alive_city
	dead_city:							# Prints out dead base
		movl $dead_base_pic, %edx
		movb (%edx), %cl
		pushl %edx
		call mp1_poke
		popl %edx
		decl %edi
		addl $2, %eax
		addl $1, %edx
		cmpl $0, %edi
		je next_city
		jmp dead_city
	next_city:
		addl $30, %eax					# Goes to next screen position
		sarl $8, %ebx
		decl %esi
		cmpl $0, %esi					# Checks if all 3 bases have been printed
		jne cities_start
    popl %edi
    popl %esi
    popl %ebx
    leave
    ret

# draw_crosshair
# Draws crosshair onto screen
draw_crosshair:
	pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
	movb $43, %cl						# Prints crosshair by finding screen position and adding its symbol
	movl crosshairs_x, %eax
	imull $2, %eax
	movl crosshairs_y, %ebx
	imull $160, %ebx
	addl %ebx, %eax
	call mp1_poke
    popl %edi
    popl %esi
    popl %ebx
    leave
    ret

# void mp1_ioctl_startgame()
# Initializes initial values and starts game
mp1_ioctl_startgame:
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		movl $0, mp1_missile_list				# Pushes inital values into variables
		movl $0xffffffff, base_alive                  
		movl $0, mp1_score
		movl $40, crosshairs_x
		movl $12, crosshairs_y
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# void mp1_ioctl_addmissile(struct missile* user_missile)
# Adds missile to linked list
mp1_ioctl_addmissile:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
	movl 8(%ebp), %ebx
    cmpl $0, %ebx                     
    je add_invalid                  # Checks if arg was valid                  
    pushl $33                       # Allocates memory
    call mp1_malloc                                     
    cmpl $0, %eax
    je add_invalid					# Checks if memory was allocated
    pushl $33                  		# Copy from user (pushes args to stack)                            
    pushl %ebx 
    pushl %eax
    call mp1_copy_from_user 					
    cmpl $0, %eax
    je add_success  				# Checks if successfully copied
	popl %eax 
	popl %ebx                    
	pushl %eax
	call mp1_free					# Frees created memory if not successful
	popl %eax                       
	movl $-1, %eax                               
	jmp add_finish 
	add_success:						# Adds missile to beginning of list
		popl %ecx
		popl %ebx
		movl mp1_missile_list, %esi 	
		movl %esi, NEXT(%ecx)
		movl %ecx, mp1_missile_list
		movl $0, %eax
		jmp add_finish
	add_invalid:
		movl $-1, %eax
	add_finish:    
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret

# void mp1_ioctl_movexhairs(unsigned long xhair_delta_packed)
# Moves and updates crosshair after it has been moved
mp1_ioctl_movexhairs:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi                   
    imull $2, crosshairs_x, %eax		# Prints crosshair
    imull $160, crosshairs_y, %ebx
    addl %ebx, %eax
	movb $32, %cl  
    call mp1_poke
    movl 8(%ebp), %ebx                     
    addw %bx, crosshairs_x        		# Updates crosshair       
    sarl $16, %ebx                         
    addw %bx, crosshairs_y        					
	check_crosshair:        			# Checks crosshair if it is at border or not
		movl crosshairs_x, %esi                       
		cmpl $0, %esi            
		jl update_x_left
		cmpl $79, %esi 
		jg update_x_right
		movl crosshairs_y, %esi
		cmpl $0, %esi 
		jl update_y_low
		cmpl $24, %esi 
		jg update_y_high
		jmp move_finish
	update_x_left:                      # Conditions for border positions of crosshair 
		movl $0, crosshairs_x
		jmp check_crosshair
	update_x_right:
		movl $79, crosshairs_x
		jmp check_crosshair
	update_y_low:
		movl $0, crosshairs_y
		jmp check_crosshair
	update_y_high:   
		movl $24, crosshairs_y    
		jmp check_crosshair	
	move_finish:
		popl %edi
		popl %esi
		popl %ebx
		leave

# int mp1 ioctl getstatus (unsigned long* user status)
# Gets current status of the game
mp1_ioctl_getstatus:
	pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi	 
    cmpl $0, 8(%ebp)				# Checks for valid arg
    je status_invalid
	andl $0, %edx
	movl base_alive, %ebx			# Ands bit with 1 to check if base is alive or dead. Then it updates the value.
	movl %ebx, %eax					# Continually shifts bit over again to check for alive or dead
	andl $1, %eax
	addl %eax, %edx
	shl $1, %edx
	movl base_alive, %ebx
	sarl $8, %ebx
	movl %ebx, %eax
	andl $1, %eax
	addl %eax, %edx
	shl $1, %edx
	sarl $8, %ebx
	movl %ebx, %ebx
	andl $1, %eax
	addl %ebx, %edx	
	shl $17, %edx       			# Puts bits in spots 16, 17, and 18
	movl $4, %ecx					# Frees space for free memory
    pushl %ecx
	call mp1_malloc	
	popl %ecx
    cmpl $0, %eax
    je status_invalid	
	movw mp1_score, %dx				# Puts score onto value
	movl %edx, (%eax)	
	pushl $4						# Copies to user
	pushl %eax
	pushl 8(%ebp)
	call mp1_copy_to_user
	addl $12, %esp
	cmpl $0, %eax
	jne status_invalid
	jmp status_success
	status_invalid:
		movl $-1, %eax
		popl %edx
		popl %edi
		popl %esi
		leave
		ret
	status_success:
		movl $0, %eax
		popl %edx
		popl %edi
		popl %esi
		leave
		ret

# int mp1 ioctl endgame (unsigned long ignore)
# Ends the game and frees all memory
mp1_ioctl_endgame:
		pushl %ebp
		movl %esp, %ebp
		pushl %ebx
		pushl %esi
		pushl %edi
		movl mp1_missile_list, %esi
	end_loop:								# Loops through list and frees variables
		cmpl $0, %esi
		je end_finish
		call mp1_missile_remove
		jmp	end_loop
	end_finish:
		popl %edi
		popl %esi
		popl %ebx
		leave
		ret
