#include "event/RandomEvent.h"

RandomEventLibrary::RandomEventLibrary()
    : localEvents{
        {"late_message", "Late Night Message",
         "A classmate sends a stressed message asking whether you can help review notes tonight.",
         {"Reply and help", "Mute the phone"},
         {Attributes(-2, -6, 4, 5, 0), Attributes(3, 0, 0, -3, 0)}, false},
        {"campus_flyer", "Campus Flyer",
         "You spot a flyer for a short workshop. The topic sounds useful but it takes part of the afternoon.",
         {"Join the workshop", "Ignore it and keep your schedule"},
         {Attributes(-1, -8, 6, 2, 0), Attributes(1, 0, 0, 0, 0)}, false},
        {"llm_hook_stub", "Unexpected Conversation",
         "A student starts an open-ended conversation that would benefit from a future LLM-generated branch.\n\n"
         "Fallback event: they ask whether campus life should be planned carefully or explored more freely.",
         {"Answer carefully", "Answer casually"},
         {Attributes(2, -2, 2, 4, 0), Attributes(4, 0, 0, 3, 0)}, true}
    }
    , llmHint("Future LLM hook: generate a dynamic campus dilemma here; currently using local fallback events.") {
}

const std::vector<RandomEventDefinition>& RandomEventLibrary::events() const {
    return localEvents;
}

const std::string& RandomEventLibrary::llmFallbackHint() const {
    return llmHint;
}
