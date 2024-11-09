# ExternalOverlayTemplate

This is a template for creating an external overlay for game cheats.

Note that this project isn't meant to be an out-of-the-box cheat, stop being a dummy and build your own cheat on top of it.

If you have any suggestions or improvements, feel free to open an issue or a pull request.

## Usage

Simply run this project in Visual Studio 2022, build and run!

## Getting Started

Navigate to src/main.cpp on line 183:

```cpp
// DEBUG: Draw a red circle at (1000, 300)
ImGui::GetBackgroundDrawList()->AddCircleFilled({ 1000, 300 }, 10.0f, ImColor(1.0f, 0.0f, 0.0f));

/*
* Cheat logic goes here then render...
*
* For example, for a kernel driver cheat you would:
* - Run your cheat
* - Talk to a driver
* - Get the data you need
* - Render the data
*
* For a usermode cheat you would:
* - Run your cheat
* - Get the data you need
* - Render the data
*/

ImGui::Render();
```

On line 183, you can see a debug red circle being drawn at (1000, 300), this is a test to see if the overlay is working.

I encourage you to modularize your code and create a class for your cheat logic, it's also a good idea to keep main.cpp as clean as possible with few lines of code.

A sample user-space logic for a cheat would be:

```cpp
// Sample data representing entities on the screen
struct Entity {
    ImVec2 position;  // Screen position of the entity
    int health;       // Health value
    bool isEnemy;     // True if entity is an enemy
};

// Sample list of entities (normally you would retrieve this data from a legitimate data source)
std::vector<Entity> entities = {
    { {800, 400}, 100.0f, true },
    { {500, 600}, 75.0f, false },
    { {1100, 450}, 50.0f, true },
    { {700, 500}, 90.0f, false }
};

// Overlay rendering logic
for (const auto& entity : entities) {
    ImColor color = entity.isEnemy ? ImColor(1.0f, 0.0f, 0.0f) : ImColor(0.0f, 1.0f, 0.0f);  // Red for enemies, green for allies
    ImVec2 textPos = { entity.position.x, entity.position.y - 20.0f };  // Position text slightly above the entity

    // Draw a circle on the entity's position
    ImGui::GetBackgroundDrawList()->AddCircleFilled(entity.position, 15.0f, color);

    // Draw health bar above the entity
    float healthBarWidth = 40.0f * (entity.health / 100.0f);  // Scale health bar width by entity's health
    ImVec2 healthBarStart = { entity.position.x - 20.0f, entity.position.y - 30.0f };
    ImVec2 healthBarEnd = { healthBarStart.x + healthBarWidth, healthBarStart.y + 5.0f };
    ImGui::GetBackgroundDrawList()->AddRectFilled(healthBarStart, healthBarEnd, color);

    // Display entity health text
    std::string healthText = "HP: " + std::to_string(static_cast<int>(entity.health));
    ImGui::GetBackgroundDrawList()->AddText(textPos, ImColor(1.0f, 1.0f, 1.0f), healthText.c_str());

    // Render the frame
    ImGui::Render();
}
```

If you wanna check real-world full example, check out https://github.com/TKazer/CS2_External/tree/master/CS2_External

## References

- [ImGui](https://github.com/ocornut/imgui)
- [MSDN](https://docs.microsoft.com/en-us/cpp/standard-library/)]

## LICENSE

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE.txt) file for more information.