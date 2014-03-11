#include <stdlib.h>
#include <err.h>

#include <nfc/nfc.h>

#include <node/v8.h>
#include <node/node.h>
#include <node/node_buffer.h>

using namespace v8;
using namespace node;

static nfc_context *context;
static nfc_device *device;

bool running = false;

static const Local<Object> JSGlobal = Context::GetCurrent()->Global();
static const Local<Function> JSBuffer = Local<Function>::Cast(JSGlobal->Get(String::New("Buffer")));
static const Local<Function> JSError = Local<Function>::Cast(JSGlobal->Get(String::New("Error")));

static const nfc_modulation mifare = {
    .nmt = NMT_ISO14443A,   // NFC modulation type
    .nbr = NBR_106          // NFC baud rate
};

struct Baton {
    nfc_target target;
    Persistent<Function> callback;
    int result;
};

void Read(Baton *baton);
void LoopRead(uv_work_t *work);
void AfterRead(uv_work_t *work);

void CheckIfRemoved(Baton *baton);
void LoopCheckIfRemoved(uv_work_t *work);
void AfterRemoved(uv_work_t *work);

void DoStop(uv_work_t *work);
void AfterStop(uv_work_t *work);

void Read(Baton *baton) {
    uv_work_t *work = new uv_work_t();
    work->data = baton;
    uv_queue_work(uv_default_loop(), work, LoopRead, (uv_after_work_cb) AfterRead);
}

void LoopRead(uv_work_t* work) {
    Baton *baton = static_cast<Baton *>(work->data);
    baton->result = nfc_initiator_select_passive_target(device, mifare, NULL, 0,
                                                        &baton->target);
}

void AfterRead(uv_work_t* work) {
    Baton *baton = static_cast<Baton *>(work->data);
    
    if (baton->result > 0) {
        nfc_iso14443a_info info = baton->target.nti.nai;
        int32_t length = (int32_t) info.szUidLen;
        
        node::Buffer *uid = node::Buffer::New(length);
        memcpy(Buffer::Data(uid), info.abtUid, length);
        
        Handle<Value> buffer[3] = {
            uid->handle_,
            Integer::New(length),
            Integer::New(0)
        };
        
        Handle<Value> argv[2] = {
            String::New("detect"),
            JSBuffer->NewInstance(3, buffer)
        };
        
        MakeCallback(baton->callback, "emit", 2, argv);
        
        CheckIfRemoved(baton);
    }
    
    delete work;
}

void CheckIfRemoved(Baton *baton) {
    uv_work_t *work = new uv_work_t();
    work->data = baton;
    uv_queue_work(uv_default_loop(), work, LoopCheckIfRemoved, (uv_after_work_cb) AfterRemoved);
}

void LoopCheckIfRemoved(uv_work_t *work) {
    while (running && 0 == nfc_initiator_target_is_present(device, NULL)) {
    }
}

void AfterRemoved(uv_work_t *work) {
    Baton *baton = static_cast<Baton *>(work->data);
    Handle<Value> argv[1] = {
        String::New("remove")
    };
    
    MakeCallback(baton->callback, "emit", 1, argv);
    
    if (running) {
        Read(baton);
    }
    
    delete work;
}

void DoStop(uv_work_t *work) {
    nfc_close(device);
    nfc_exit(context);
}

void AfterStop(uv_work_t *work) {
    Baton *baton = static_cast<Baton *>(work->data);
    
    Handle<Value> argv[1] = {
        String::New("close")
    };
    
    MakeCallback(baton->callback, "emit", 1, argv);
    
    delete work;
}

struct NfcId: ObjectWrap {
    static Handle<Value> New(const Arguments& args);
    static Handle<Value> Listen(const Arguments& args);
    static Handle<Value> Close(const Arguments& args);
    static Handle<Value> Version(const Arguments& args);
};

Handle<Value> NfcId::New(const Arguments& args) {
    HandleScope scope;
    
    assert(args.IsConstructCall());
    
    NfcId* self = new NfcId();
    self->Wrap(args.This());
    
    return scope.Close(args.This());
}

Handle<Value> NfcId::Listen(const Arguments& args) {

    running = true;
    
    Baton* baton = new Baton();
    
    Handle<Function> callback = Handle<Function>::Cast(args.This());
    baton->callback = Persistent<Function>::New(callback);
    
    nfc_init(&context);
    
    if (context == NULL) {
        Handle<Value> error[1] = {
            String::New("Failed to initialize NFC context.")
        };
        
        Handle<Value> argv[2] = {
            String::New("error"),
            JSError->NewInstance(1, error)
        };
        
        MakeCallback(baton->callback, "emit", 2, argv);
        return Undefined();
    }
    
    device = nfc_open(context, NULL);
    
    if (device == NULL) {
        nfc_exit(context);
        
        Handle<Value> error[1] = {
            String::New("Failed to open NFC device.")
        };
        
        Handle<Value> argv[2] = {
            String::New("error"),
            JSError->NewInstance(1, error)
        };
        
        MakeCallback(baton->callback, "emit", 2, argv);
        return Undefined();
    }
    
    if (nfc_initiator_init(device) < 0) {
        nfc_close(device);
        nfc_exit(context);
        
        Handle<Value> error[1] = {
            String::New("Failed to initialize NFC initiator.")
        };
        
        Handle<Value> argv[2] = {
            String::New("error"),
            JSError->NewInstance(1, error)
        };
        
        MakeCallback(baton->callback, "emit", 2, argv);
        return Undefined();
    }
    
    Read(baton);
    
    Handle<Value> argv[1] = {
        String::New("listenning")
    };
    
    MakeCallback(baton->callback, "emit", 1, argv);
    
    return Undefined();
}

Handle<Value> NfcId::Close(const Arguments &args) {
    running = false;
    
    Baton* baton = new Baton();
    
    Handle<Function> callback = Handle<Function>::Cast(args.This());
    baton->callback = Persistent<Function>::New(callback);
    
    uv_work_t *work = new uv_work_t();
    work->data = baton;
    uv_queue_work(uv_default_loop(), work, DoStop, (uv_after_work_cb) AfterStop);
    
    return Undefined();
}

Handle<Value> NfcId::Version(const Arguments &args) {
    return String::New(nfc_version());
}

extern "C" void Initialize(Handle<Object> exports, Handle<Object> module) {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(NfcId::New);
    
    tmpl->InstanceTemplate()->SetInternalFieldCount(1);
    tmpl->SetClassName(String::New("NfcId"));
    
    NODE_SET_PROTOTYPE_METHOD(tmpl, "listen", NfcId::Listen);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "close", NfcId::Close);
    
    NODE_SET_METHOD(tmpl, "version", NfcId::Version);
    
    module->Set(String::NewSymbol("exports"), tmpl->GetFunction());
}

NODE_MODULE(nfc, Initialize)
