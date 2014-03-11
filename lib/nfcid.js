var EventEmitter = require('events').EventEmitter
  , inherits = require('util').inherits

var Binding = require('../build/Release/nfc.node')

module.exports = NfcId
module.exports.version = Binding.version()

inherits(NfcId, EventEmitter)

function NfcId (callback) {
  if (!(this instanceof NfcId)) {
    var nfc = new NfcId()

    if ('function' === typeof callback) {
      nfc.on('detect', callback)
    }

    return nfc
  }

  this.binding = new Binding()
  this.binding.emit = this.emit.bind(this)

  EventEmitter.call(this)
}

NfcId.prototype.listen = function (callback) {
  if ('function' === typeof callback) {
    this.on('listenning', callback)
  }

  this.binding.listen()

  return this
}

NfcId.prototype.close = function (callback) {
  var self = this

  if ('function' === typeof callback) {
    self.on('close', callback)
  }

  self.once('close', function () {
    self.removeAllListeners()
  })

  self.binding.close()

  return this
}
