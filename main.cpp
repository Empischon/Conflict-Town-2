#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <raylib.h>
#include <vector>

struct Shell {
  Vector2 position{ 0.0f, 0.0f };
  Vector2 velocity{ 0.0f, 0.0f };
  
  Shell(const Vector2& other_position, const Vector2& velocity_from_direction)
  {
    position = other_position;
    velocity = velocity_from_direction;
    velocity.x *= 5.0f;
    velocity.y *= 5.0f;
  }
};

struct Tank {
  int redirection_timer{0};
  int reload_timer{0};
  int direction{ 7 };
  Vector2 velocity{ 0.0f, 0.0f };
  Rectangle position{ 0.0f,0.0f,150.0f,150.0f };
  Rectangle collision{ 50.0f,50.0f,50.0f,50.0f};
};

Vector2 direction_to_vel(const int direction) {
  switch(direction) {
    case 0:
      return {-1.0f, 0.0f};
    case 1:
      return {-1.0f, 1.0f};
      
    case 2:
      return {0.0f, 1.0f};
    case 3:
      return {1.0f, 1.0f};
      
    case 4:
      return {1.0f, 0.0f};
    case 5:
      return {1.0f, -1.0f};
      
    case 6:
      return {0.0f, -1.0f};
    case 7:
      return {-1.0f, -1.0f};
  }
  
  return {0.0f,0.0f};
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    std::srand(std::time(nullptr));

    InitWindow(screenWidth, screenHeight, "Conflict Town 2");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    //Texture2D scarfy = LoadTexture("resources/Battle_City_JPN_General_Sprites.png");        // Texture loading
    std::vector<Texture2D> player_tank_images;
    std::vector<Texture2D> enemy_tank_images;
    Rectangle tank_texture_rec{ 0.0f,0.0f,150.0f,150.0f };
    Rectangle tank_collision_rec{ 350.0f,350.0f,50.0f,50.0f };
    Rectangle player_position{ 300.0f, 300.0f, 150.0f,150.0f };
    Vector2 origin{ 0.0f, 0.0f };
    float rotation{ 0.0f };
    
    // Asset loading
    for(int i = 1; i < 9; ++i) {
      std::string filename_string = "resources/Tiger_export/Merged/german_tiger_merged";
      filename_string = filename_string + std::to_string(i);
      filename_string = filename_string + ".png";
      player_tank_images.push_back( LoadTexture( filename_string.c_str() ) );
    }
    
    for(int i = 1; i < 9; ++i) {
      std::string filename_string = "resources/Panzer4_export/Merged/german_panzer4_merged";
      filename_string = filename_string + std::to_string(i);
      filename_string = filename_string + ".png";
      enemy_tank_images.push_back( LoadTexture( filename_string.c_str() ) );
    }
    
    Texture2D fireworks = LoadTexture( "resources/New_All_Fire_Bullet_Pixel_16x16/All_Fire_Bullet_Pixel_16x16_00.png" );
    constexpr int shell_pixel_size{16};
    Rectangle shell_image_rec{ 14 * shell_pixel_size, 18 * shell_pixel_size, shell_pixel_size, shell_pixel_size };
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    
    int player_direction{ 7 };
    int player_reload{ 0 };
    Vector2 player_velocity{ 0.0f, 0.0f};
    std::vector<Shell> shells;
    std::vector<Tank> enemies;
    enemies.push_back({});
    enemies.back().redirection_timer = (rand() % 60) + 50;
    enemies.back().reload_timer = (rand() % 100) + 100;
    
    bool defeat_flag{false};
    while( !WindowShouldClose() )
    {
      // Handle Input
      if ( !defeat_flag) {
        if ( IsKeyDown(KEY_LEFT) and not ( IsKeyDown(KEY_UP) or IsKeyDown(KEY_DOWN) ) ) player_direction = 0;
        if ( IsKeyDown(KEY_LEFT) and IsKeyDown(KEY_DOWN) ) player_direction = 1;
        
        if ( IsKeyDown(KEY_DOWN) and not ( IsKeyDown(KEY_LEFT) or IsKeyDown(KEY_RIGHT) ) ) player_direction = 2;
        if ( IsKeyDown(KEY_DOWN) and IsKeyDown(KEY_RIGHT) ) player_direction = 3;
        
        if ( IsKeyDown(KEY_RIGHT) and not ( IsKeyDown(KEY_UP) or IsKeyDown(KEY_DOWN) ) ) player_direction = 4;
        if ( IsKeyDown(KEY_RIGHT) and IsKeyDown(KEY_UP) ) player_direction = 5;
        
        if ( IsKeyDown(KEY_UP) and not ( IsKeyDown(KEY_LEFT) or IsKeyDown(KEY_RIGHT) ) ) player_direction = 6;
        if ( IsKeyDown(KEY_UP) and IsKeyDown(KEY_LEFT) ) player_direction = 7;
        
        if( IsKeyDown(KEY_UP) and not IsKeyDown(KEY_DOWN) ) player_velocity.y = -2.0f;
        if( not IsKeyDown(KEY_UP) and IsKeyDown(KEY_DOWN) ) player_velocity.y = 2.0f;
        if( not IsKeyDown(KEY_UP) and not IsKeyDown(KEY_DOWN) ) player_velocity.y = 0.0f;
        if( IsKeyDown(KEY_LEFT) and not IsKeyDown(KEY_RIGHT) ) player_velocity.x = -2.0f;
        if( not IsKeyDown(KEY_LEFT) and IsKeyDown(KEY_RIGHT) ) player_velocity.x = 2.0f;
        if( not IsKeyDown(KEY_LEFT) and not IsKeyDown(KEY_RIGHT) ) player_velocity.x = 0.0f;
        
        if( IsKeyDown(KEY_SPACE) and player_reload <= 0 ) {
          Vector2 temp_position;
          Vector2 velocity_from_direction = direction_to_vel(player_direction);
          
          temp_position.x = player_position.x + player_position.width/2.0f + (velocity_from_direction.x * 50);
          temp_position.y = player_position.y + player_position.height/2.0 + (velocity_from_direction.y * 50);
          
          shells.emplace_back( temp_position, velocity_from_direction );
          player_reload = 31;
        }
      }
      
      // Update State
      if ( !defeat_flag) {
        if (player_reload > 0) --player_reload;
        
        if (player_position.x + player_velocity.x > 0 and player_position.x + player_velocity.x + 150 < 800) {
          player_position.x += player_velocity.x;
          tank_collision_rec.x += player_velocity.x;
        }
        if (player_position.y + player_velocity.y > 0 and player_position.y + player_velocity.y + 150 < 450) {
          player_position.y += player_velocity.y;
          tank_collision_rec.y += player_velocity.y;
        }
        
        for(auto& shell : shells)
        {
          shell.position.x += shell.velocity.x;
          shell.position.y += shell.velocity.y;
        }
        
        for(size_t ii = shells.size(); ii >0; --ii)
        {
          size_t i = ii-1;
          if( shells[i].position.x < 20 or shells[i].position.x > 780 or shells[i].position.y < 20 or shells[i].position.y > 430) shells.erase( shells.begin() + i );
          
          if( CheckCollisionCircleRec( shells[i].position, 8.0f, tank_collision_rec ) ) { defeat_flag = true; }
          
          bool temp_flag{false};
          for(size_t j = 0; j < enemies.size(); ++j)
          {
            if( CheckCollisionCircleRec( shells[i].position, 8.0f, enemies[j].collision ) ) 
            {
              enemies.erase( enemies.begin() + j);
              shells.erase( shells.begin() + i );
              
              
              for(int k = 0; k < 2; k++) {
                enemies.emplace_back();
                enemies.back().direction = (int) (rand() % 8);
                enemies.back().velocity = direction_to_vel(enemies.back().direction);
                enemies.back().velocity.x *= 3.0f;
                enemies.back().velocity.y *= 3.0f;
                enemies.back().position.x = rand() % 650;
                enemies.back().position.y = rand() % 300;
                enemies.back().collision.x = enemies.back().position.x+50;
                enemies.back().collision.y = enemies.back().position.y+50;
                enemies.back().redirection_timer = (rand() % 60) + 40;
                enemies.back().reload_timer = (rand() % 100) + 100;
                printf("Enemy added at %f %f velocity %f %f direction %i\n", enemies.back().position.x, enemies.back().position.y, enemies.back().velocity.x, enemies.back().velocity.y, enemies.back().direction);
              }
              temp_flag = true;
            }
            if(temp_flag) break;
          }
        }
        
        for(auto& enemy : enemies) 
        {
          --enemy.redirection_timer;
          --enemy.reload_timer;
          //printf("enemy timers: %i %i\n", enemy.redirection_timer, enemy.reload_timer);
          
          if (enemy.position.x + enemy.velocity.x > 0 and enemy.position.x + enemy.velocity.x + 150 < 800) {
            enemy.position.x  += enemy.velocity.x;
            enemy.collision.x += enemy.velocity.x;
          }
          if (enemy.position.y + enemy.velocity.y > 0 and enemy.position.y + enemy.velocity.y + 150 < 450) {
            enemy.position.y  += enemy.velocity.y;
            enemy.collision.y += enemy.velocity.y;
          }
          if (enemy.position.x < 10 or enemy.position.x > 640 or enemy.position.y < 10 or enemy.position.y > 440)
          {
            printf("Early redirection triggered at %f %f\n", enemy.position.x, enemy.position.y);
            enemy.redirection_timer = 0;
          }
        
          if( enemy.redirection_timer <= 0 )
          {
            enemy.direction = (int) (rand() % 8);
            
            /*while ( ( enemies.back().direction == 7 or enemies.back().direction == 0 or enemies.back().direction == 1 ) and 
                  ( enemies.back().position.x < 20 ) )
            {
              enemies.back().direction = (int) (rand() % 8);
            }
            
            while ( ( enemies.back().direction == 3 or enemies.back().direction == 4 or enemies.back().direction == 5 ) and 
                  ( enemies.back().position.x > 780 ) )
            {
              enemies.back().direction = (int) (rand() % 8);
            }
            
            while ( ( enemies.back().direction == 5 or enemies.back().direction == 6 or enemies.back().direction == 7 ) and 
                  ( enemies.back().position.y < 20 ) )
            {
              enemies.back().direction = (int) (rand() % 8);
            }
            
            while ( ( enemies.back().direction == 1 or enemies.back().direction == 2 or enemies.back().direction == 3 ) and 
                  ( enemies.back().position.y > 430 ) )
            {
              enemies.back().direction = (int) (rand() % 8);
            }*/
            
            enemy.velocity = direction_to_vel(enemy.direction);
            enemy.velocity.x *= 3.0f;
            enemy.velocity.y *= 3.0f;
            printf("position %f %f new direction: %i, new velocity %f %f\n",  enemy.position.x, enemy.position.y, enemy.direction, enemy.velocity.x, enemy.velocity.y);
            enemy.redirection_timer = (rand() % 60) + 50;
          }
          
          if( enemy.reload_timer <= 0 )
          {
            Vector2 temp_position;
            Vector2 velocity_from_direction = direction_to_vel(enemy.direction);
            
            temp_position.x = enemy.position.x + enemy.position.width/2.0f + (velocity_from_direction.x * 50);
            temp_position.y = enemy.position.y + enemy.position.height/2.0 + (velocity_from_direction.y * 50);
            
            shells.emplace_back( temp_position, velocity_from_direction );
            enemy.reload_timer = (rand() % 100) + 100;
          }
        }
      }
      
      
      // Draw
      BeginDrawing();
      ClearBackground(RAYWHITE);
      if(defeat_flag) DrawText("DEFEAT", 200, 100, 30, BLACK);
      
      DrawTexturePro( player_tank_images[player_direction], tank_texture_rec, player_position, origin, rotation, WHITE );
      DrawRectangleLinesEx(tank_collision_rec, 2, RED);
      
      for(const auto& enemy : enemies)
      {
        DrawTexturePro( enemy_tank_images[enemy.direction], tank_texture_rec, enemy.position, origin, rotation, WHITE );
        DrawRectangleLinesEx(enemy.collision, 2, RED);
      }
      
      for(const auto& shell : shells)
      {
        Rectangle temp_position{ shell.position.x, shell.position.y, shell_pixel_size, shell_pixel_size };
        DrawTexturePro( fireworks, shell_image_rec, temp_position, origin, rotation, WHITE );
      }
      EndDrawing();
    }
}