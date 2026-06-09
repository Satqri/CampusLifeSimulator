#include "core/RandomEvent.h"

RandomEventLibrary::RandomEventLibrary()
    : localEvents{
        {"late_message", "Late Night Message",
         "A classmate sends a stressed message asking whether you can help review notes tonight.",
         {"Reply and help", "Mute the phone"},
         {Attributes{.energy = -6, .san = -2, .academic = 4, .social = 5}, Attributes{.san = 3, .social = -3}}, false},
        {"campus_flyer", "Campus Flyer",
         "You spot a flyer for a short workshop. The topic sounds useful but it takes part of the afternoon.",
         {"Join the workshop", "Ignore it and keep your schedule"},
         {Attributes{.energy = -8, .san = -1, .academic = 6, .social = 2}, Attributes{.san = 1}}, false},
        {"llm_hook_stub", "Unexpected Conversation",
         "A student starts an open-ended conversation that would benefit from a future LLM-generated branch.\n\n"
         "Fallback event: they ask whether campus life should be planned carefully or explored more freely.",
         {"Answer carefully", "Answer casually"},
         {Attributes{.energy = -2, .san = 2, .academic = 2, .social = 4}, Attributes{.san = 4, .social = 3}}, true}
    }
    , llmHint("Future LLM hook: generate a dynamic campus dilemma here; currently using local fallback events.") {
}

const std::vector<RandomEventDefinition>& RandomEventLibrary::events() const {
    return localEvents;
}

const std::string& RandomEventLibrary::llmFallbackHint() const {
    return llmHint;
}
