
#include <Atomic/UI/UIEvents.h>

#include "JSVM.h"
#include "JSEventHelper.h"

namespace Atomic
{

JSEventHelper::JSEventHelper(Context* context) :
    Object(context)
{
}

JSEventHelper::~JSEventHelper()
{

}

void JSEventHelper::AddEventHandler(StringHash eventType)
{
    SubscribeToEvent(eventType, HANDLER(JSEventHelper, HandleEvent));
}

void JSEventHelper::AddEventHandler(Object* sender, StringHash eventType)
{
    SubscribeToEvent(sender, eventType, HANDLER(JSEventHelper, HandleEvent));
}

void JSEventHelper::HandleEvent(StringHash eventType, VariantMap& eventData)
{    

    JSVM* vm = JSVM::GetJSVM(0);
    duk_context* ctx = vm->GetJSContext();

    duk_idx_t top = duk_get_top(ctx);

    js_push_class_object_instance(ctx, this);

    duk_get_prop_string(ctx, -1, "__eventHelperFunctions");

    assert(duk_is_object(ctx, -1));

    duk_get_prop_string(ctx, -1, eventType.ToString().CString());

    if (duk_is_function(ctx, -1))
    {
        assert(duk_is_object(ctx, -1));

        //TODO: this is expensive, we should be caching these in VM
        // and reusing, instead of creating new variant map object
        // per event handler
        js_push_variantmap(ctx, eventData);

        if (duk_pcall(ctx, 1) != 0)
        {
            vm->SendJSErrorEvent();
        }
        else
        {
            // For widget events, need to check return value
            // and set whether handled
            if (eventType == E_WIDGETEVENT)
            {
                if (duk_is_boolean(ctx, -1))
                {
                    if (duk_to_boolean(ctx, -1))
                    {
                        eventData[WidgetEvent::P_HANDLED] = true;
                    }
                }
            }
        }
    }

    duk_set_top(ctx, top);

}

}
