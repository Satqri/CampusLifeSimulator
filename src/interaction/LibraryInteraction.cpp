#include "interaction/LibraryInteraction.h"
#include "core/LibraryConfig.h"
#include "core/GameContext.h"
#include <sstream>

namespace LibraryInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    const std::string& id = ip.actionId;

    if (id.rfind("library_shelf_", 0) == 0) {
        ctx.selectedLibraryBook = std::clamp(id.back() - '0', 0, 3);
        std::ostringstream body;
        body << ip.label << " selected " << ctx.libraryBooks[ctx.selectedLibraryBook].name
             << ". Reading progress: " << ctx.libraryBookProgress[ctx.selectedLibraryBook]
             << "%. Browse Shelf does not consume time.";
        ctx.activityNotice.show("Shelf Browsed", body.str());
        return true;
    }

    if (id == "library_table") {
        const int book = ctx.selectedLibraryBook;
        ctx.libraryBookProgress[book] = std::min(100, ctx.libraryBookProgress[book] + 25);
        const Attributes& delta = ctx.libraryBooks[book].delta;

        std::ostringstream body;
        body << "Read " << ctx.libraryBooks[book].name << " for 30 minutes. "
             << ctx.libraryBooks[book].skill << " progress is now "
             << ctx.libraryBookProgress[book] << "%.";
        ctx.runTimedActivity(30, delta, "Reading Complete", body.str());
        return true;
    }

    return false;
}

} // namespace LibraryInteraction
