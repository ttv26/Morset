#include "ui.h"
#include "audio.h"
#include "trainer.h"
#include "stats.h"
#include <iostream>
#include <cstdlib>

int main() {
    ui::initTerminal();
    audio::init();

    stats::StatsManager statsManager;
    trainer::Trainer trainerInstance(statsManager);

    while (true) {
        ui::clearScreen();
        ui::showBanner();

        std::vector<std::string> menuOptions = {
            "Text → Morse  (learn to encode)",
            "Morse → Text  (learn to decode)",
            "Listen & Type (audio quiz)",
            "Custom Practice (pick your characters)",
            "Free Encode   (type anything, hear Morse)",
            "Morse Reference Chart",
            "Statistics & Progress",
            "Settings",
            "Exit"
        };

        int choice = ui::showMenu("Main Menu", menuOptions);

        switch (choice) {
            case 1: trainerInstance.textToMorse(); break;
            case 2: trainerInstance.morseToText(); break;
            case 3: trainerInstance.listenAndType(); break;
            case 4: trainerInstance.customPractice(); break;
            case 5: trainerInstance.freeEncode(); break;
            case 6: ui::showMorseChart(); break;
            case 7: statsManager.showDashboard(); break;
            case 8: trainerInstance.configure(); break;
            case 9:
                statsManager.endSession();
                ui::clearScreen();
                std::cout << ui::colorGreen("\n  Thanks for practicing! 73!\n\n");
                return 0;
        }
    }

    return 0;
}
