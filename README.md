node-nfcid
========

A Node.js binding of [libnfc][1] that detects card UIDs.


## Installation

First of all, you may have [libnfc][1] installed. Check out [here][2] for
introduction.

After that, you may install this module using `npm install`:

```
$ npm install libnfc
```


## Prerequisites

* node (>= 0.10.X)
* [libnfc][1] (>= 1.7.X)
* libusb
* Compatible card reader with driver installed (e.g. ACR122U with _libasccid_)


## Quick Start

```js
require('nfcid')(function (uid) {
  console.log('nfc uid detect: %s', uid)
}).listen()
```


## API Doc

### nfcid([callback])

Shortcut for initializing and starting an instance, returns an
[Nfc](#class-libnfcnfc) instance, which is an instance of
[`events.EventEmitter`][3].

Equals to following code:

```js
var nfcid = require('nfcid')

var nfc = new nfcid()
nfc.on('detect', function (uid) {
  // ...
})
```

Optionally you can pass a `callback` as an listener for the
[`'detect'`](#event-detect) event.


### Class: nfcid.Nfc

The Nfc Class.


#### new nfcid.Nfc()

Construct a new Nfc instance.


#### nfc.listen([callback])

Start polling for card.

You can also pass a `callback` as an listener for the
[`'listenning'`](#event-listenning) event.


#### nfc.close([callback])

Stop polling and detach all listeners, so that the process could safely exit.

You can also pass a `callback` as an listener for the [`'close'`](#event-close)
event, which would be emitted once the polling is fully ended. After this
event is emitted all listeners would be detached.


#### nfc.version()

Get which version of libnfc the module compiled with.


#### Event: 'listenning'

Emitted when the polling process is successfully started and listenning for
cards.


#### Event: 'detect'

* `uid` String - the unique ID of the card detected.

Emitted when a compatible card is placed to the reader and detected.


#### Event: 'remove'

Emitted when the card is removed from the reader.


#### Event: 'error'

* `err` Error

Emitted when an error occurs. The [`'close'`](#event-close) event will be
called directly following this event.


#### Event: 'close'

* `hadError` Boolean - `true` if the polling process is ended due to an error.

Emitted once the polling process is fully ended. After this event is emitted
all listeners would be detached so that the process could safely exit.


## Credit

Thanks to the [original binding][4] from cammo Tapia and a Node.js 0.10.X
[patch][5] from LambdaDriver.


## Hacking

Please feel free to make patches or ask through [New Issue][6].


## License

This module is available under the terms of the [MIT License][7].


[1]: https://code.google.com/p/libnfc/
[2]: http://nfc-tools.org/index.php?title=Libnfc
[3]: http://nodejs.org/api/events.html#events_class_events_eventemitter
[4]: https://github.com/camme/node-nfc
[5]: https://github.com/LambdaDriver/node-nfc
[6]: https://github.com/xingrz/node-nfcid/issues/new
[7]: https://github.com/xingrz/node-nfcid/blob/master/LICENSE
