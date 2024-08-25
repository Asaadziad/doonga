typedef enum {
	ENTITY_player,
	ENTITY_fire,
	ENTITY_mage,
	ENTITY_range,
	ENTITY_melee
} EntityType;

typedef enum {
	SPRITE_player,
	SPRITE_fire,
	SPRITE_range,
	SPRITE_melee,
	SPRITE_mage,
	SPRITE_MAX,
} SpriteType;

typedef struct Sprite {
	Gfx_Image* sprite;
} Sprite;

typedef struct Entity {
	bool	   isValid;
	EntityType type;
	Vector2    pos;

	u64 spriteId;
	bool isNpc;
} Entity;
// :entity

typedef struct WorldFrame {
	int level;
} WorldFrame;

#define MAX_ENTITIES 4 * 1024

typedef struct World {
	Entity entities[MAX_ENTITIES];
	Sprite sprites[SPRITE_MAX];
	WorldFrame word_frame; 
} World;



void load_world_entities(Entity* entities, int level){
	entities[0] = (Entity){.pos = v2(0,0), .type=ENTITY_player, .spriteId=SPRITE_player, .isValid=true, .isNpc=false};
	int enemies_to_spawn = get_random_int_in_range(0 + 10 * (level - 1), 10 * level);
	
	for(int i = 1; i < enemies_to_spawn; i++) {
		float64 random_float = get_random_float64_in_range(0, i);
		int dice = (int)random_float % 3;	
		float64 random_pos_range_x = get_random_float64_in_range(-200,200);
		float64 random_pos_range_y = get_random_float64_in_range(-200,200);
		switch(dice){
			case 0:
			entities[i] = (Entity){.pos= v2(random_pos_range_x,random_pos_range_y), .type=ENTITY_melee, .spriteId=SPRITE_melee, .isValid=true, .isNpc=true};
			break;
			case 1:
			entities[i] = (Entity){.pos= v2(random_pos_range_x,random_pos_range_y), .type=ENTITY_range, .spriteId=SPRITE_range, .isValid=true,  .isNpc=true};
			break;
			case 2:
			entities[i] = (Entity){.pos= v2(random_pos_range_x,random_pos_range_y), .type=ENTITY_mage, .spriteId=SPRITE_mage, .isValid=true	 ,	 .isNpc=true};
			break;
		}
	} 

	for(int i = enemies_to_spawn; i < 512 - enemies_to_spawn; i++) {
		float64 random_pos_range_x = get_random_float64_in_range(-200,200);
		float64 random_pos_range_y = get_random_float64_in_range(-200,200);
		entities[i] = (Entity){.pos= v2(random_pos_range_x,random_pos_range_y), .type=ENTITY_fire, .spriteId=SPRITE_fire, .isValid=true,  .isNpc=false};
	}
}

Vector2 get_sprite_size(Sprite* sprite){
	return (Vector2){.x=sprite->sprite->width, .y=sprite->sprite->height};
}

void load_sprite_images(Sprite* sprites){
	Gfx_Image* player = load_image_from_disk(fixed_string("assets/player.png"), get_heap_allocator());
	Gfx_Image* fire = load_image_from_disk(fixed_string("assets/fire.png"), get_heap_allocator());
	Gfx_Image* mage = load_image_from_disk(fixed_string("assets/mage.png"), get_heap_allocator());
	Gfx_Image* range = load_image_from_disk(fixed_string("assets/range.png"), get_heap_allocator());
	Gfx_Image* melee = load_image_from_disk(fixed_string("assets/melee.png"), get_heap_allocator());
	
	sprites[SPRITE_player] = (Sprite) {.sprite=player};
	sprites[SPRITE_fire]   = (Sprite) {.sprite=fire};
	sprites[SPRITE_mage] = (Sprite)   {.sprite=mage};
	sprites[SPRITE_range]   = (Sprite){ .sprite=range};
	sprites[SPRITE_melee] = (Sprite)  {.sprite=melee};
}

float64 v2_dist(Vector2 v, Vector2 u){
	return sqrtf(powf(v.x - u.x, 2) + powf(v.y - u.y, 2));
}

void move_npc_towards_player(Entity* npc, Vector2 player_pos, float64 delta_t){
	float64 move_x = player_pos.x - npc->pos.x;
	float64 move_y = player_pos.y - npc->pos.y;
	Vector2 input  = (Vector2){.x=move_x, .y=move_y};  
	input = v2_normalize(input);
	npc->pos = v2_add(npc->pos, v2_mulf(input, 10 * delta_t));
}

int entry(int argc, char **argv) {
	
	World world = {0};
	world.word_frame.level = 1;

	window.title = STR("Doonga");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 700; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	load_sprite_images(world.sprites);	
	load_world_entities(world.entities, world.word_frame.level);
	
	Entity* player_en = &world.entities[0];
	float64 last_time = os_get_elapsed_seconds();
	while (!window.should_close) {
		reset_temporary_storage();
		os_update();
		
		// camera stuff
		{
			draw_frame.projection = m4_make_orthographic_projection(window.width * -0.5, window.width * 0.5, window.height * -0.5, window.height * 0.5, -1, 10); 
			float zoom = 5.3;
			draw_frame.camera_xform = m4_make_scale(v3(1,1,1.0));
			draw_frame.camera_xform = m4_mul(draw_frame.camera_xform, m4_make_translation(v3(player_en->pos.x, player_en->pos.y, 0)));
			draw_frame.camera_xform = m4_mul(draw_frame.camera_xform, m4_make_scale(v3(1.0 / zoom, 1.0 / zoom, 1.0)));	
		}

		float64 now = os_get_elapsed_seconds();

		float64 delta = now - last_time;
		last_time = now;

		Vector2 input = v2(0,0);
		if (is_key_down('A')) {
			input.x -= 1.0;
		}
		if (is_key_down('D')) {
			input.x += 1.0;
		}
		if (is_key_down('S')) {
			input.y -= 1.0;
		}
		if (is_key_down('W')) {
		    input.y += 1.0;
		}	
		input = v2_normalize(input);
		world.entities[0].pos = v2_add(world.entities[0].pos, v2_mulf(input, 100 * delta));
		
		for(int i = 0;i < MAX_ENTITIES;i++){
			if(world.entities[i].isValid){
				if(world.entities[i].isNpc){
					move_npc_towards_player(&world.entities[i], player_en->pos, delta);
				}
				Sprite entity_sprite = world.sprites[world.entities[i].spriteId];
				Vector2 sprite_size = get_sprite_size(&entity_sprite);
				draw_image(entity_sprite.sprite, v2(world.entities[i].pos.x - 0.5 * sprite_size.x, world.entities[i].pos.y), get_sprite_size(&entity_sprite), COLOR_WHITE);
			}
		}
		
		gfx_update();
	}

	return 0;
}