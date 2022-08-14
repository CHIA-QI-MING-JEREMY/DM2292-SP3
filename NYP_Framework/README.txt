CHIA QI MING JEREMY
214559G
DM2213 2DGC A1

Movement:
	A: 	Move left
	D: 	Move right
	SPACE:	Jump
	
	W:	Move up the rope
	S:	Move down the rope
	
	R: 	Resets the position of the player to the last visited checkpoint (consumes 1 life)

Mechanics: 
	1)	Coloured Collision
			When the player collects orbs, they are able to change their character's colour
			to the colour of the collected orb. For example, if the player has collected the
			yellow and red orbs, they can change their character's colour to either yellow
			or red.

			The colour of the character determines the tiles that they collide with. The
			character will not collide with tiles of the same colour. For example, if the
			character's colour is yellow, the character will be able to walk through yellow
			tiles, but not tiles of any other colour. If the character's colour is green,
			the character will be able to walk through green tiles, but not tiles of any
			other colour.

			When the player changes their character's colour, they will consume the orb's
			charge, and will have to visit a checkpoint to restore any spent orb charges.
			When an orb is spent, the player will not be able to change their character's
			colour to that of the orb. For example, the character is currently yellow.
			Changing the character's colour to red consumes the red orb's charge. Changing
			the character's colour to green then consumes the green orb's charge. Now, the
			player is unable to change their character's colour to red as the red orb's
			charge has been spent. By visiting a checkpoint, all spent charges will be
			restored.

	2)	Rope Climbing
			When the player interacts with the rope post, the rope post will detect if there
			is empty space on its left or right, and deploy the rope accordingly. The length
			of the rope is determined by the height of the first ground block the rope touches.
			The rope will continue extending downwards until it hits a ground block, after
			which it will stop extending downwards.

			The player is able to move up and down the rope by pressing W and S respectively.
			The player is also able to jump onto the rope and continue moving up or down.
			The player is also able to get off the rope at any height that they wish, after
			which they will fall towards the ground.			

	3)	Checkpoint System
			There are 2 types of checkpoints, black (inactive) and red (active). When the
			player interacts with a Black CP, they will activate the CP, changing it to a
			Red CP. They will also restore their health, and restore any spent orb charges.
			
			When the player activates another Black CP, the previously activated Red CP
			becomes inactive and changes to a Black CP, while the just activated CP becomes
			a Red CP.

			If the player loses all their health or presses R to reset, they will lose 1 life
			and respawn back at the Red CP with maximum health. If the player loses all their
			lives, they will immediately be brought to the LOSE screen. If the player makes it
			to the door, they will be brought to the WIN screen.