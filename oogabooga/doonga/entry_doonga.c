typedef enum {
	ENTITY_player,
	ENTITY_fire,
} EntityType;

typedef enum {
	SPRITE_player,
	SPRITE_fire,
	SPRITE_MAX,
} SpriteType;

typedef struct Sprite {
	Gfx_Image* sprite;
	Vector2    size;
} Sprite;

typedef struct Entity {
	bool	   isValid;
	EntityType type;
	Vector2    pos;

	u64 spriteId;
} Entity;

#define MAX_ENTITIES 1024

typedef struct World {
	Entity entities[MAX_ENTITIES];
	Sprite sprites[SPRITE_MAX];
} World;

void load_world_entities(Entity* entities){
	entities[0] = (Entity){.pos = v2(0,0), .type=ENTITY_player, .spriteId=SPRITE_player, .isValid=true};
	for(int i = 1; i < 11; i++){
		entities[i] = (Entity){.pos = v2(get_random_float64_in_range(-100, 100),get_random_float64_in_range(-100, 100)), .type=ENTITY_fire, .spriteId=SPRITE_fire, .isValid=true};
	}	
}

void load_sprite_images(Sprite* sprites){

	Gfx_Image* player = load_image_from_disk(fixed_string("assets/player.png"), get_heap_allocator());
	Gfx_Image* fire = load_image_from_disk(fixed_string("assets/fire.png"), get_heap_allocator());

	sprites[SPRITE_player] = (Sprite){.size=v2(6.0 , 13.0),.sprite=player};
	sprites[SPRITE_fire]   = (Sprite){.size=v2(13.0, 14.0), .sprite=fire};
}

int entry(int argc, char **argv) {
	
	World world = {0};

	window.title = STR("Doonga");
	window.scaled_width = 1280; // We need to set the scaled size if we want to handle system scaling (DPI)
	window.scaled_height = 700; 
	window.x = 200;
	window.y = 90;
	window.clear_color = hex_to_rgba(0x6495EDff);
	
	load_sprite_images(world.sprites);	
	load_world_entities(world.entities);
	
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
				Sprite entity_sprite = world.sprites[world.entities[i].spriteId];
				draw_image(entity_sprite.sprite, v2(world.entities[i].pos.x - 0.5 * entity_sprite.size.x, world.entities[i].pos.y), entity_sprite.size, COLOR_WHITE);
			}
		}
		
		gfx_update();
	}

	return 0;
}