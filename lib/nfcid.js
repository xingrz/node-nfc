var EventEmitter = require('events').EventEmitter
  , inherits = require('util').inherits

var nfc = require('../build/Release/nfc.node')

var debug = require('debug')('nfcid')

module.exports = Nfc

inherits(Nfc, EventEmitter)

function Nfc (options) {
  if (!(this instanceof Nfc)) {
    var nfc = new Nfc()

    debug('construct instance with nfcid()')

    if ('function' === typeof options) {
      nfc.on('detect', options)
    }

    debug('attached \'detect\' event from nfcid(callback) ')

    return nfc
  }

  if ('undefined' === typeof options) {
    options = {}
  }

  if ('string' === typeof options) {
    this.executable = options
    debug('use custom executable %s', options)
  } else {
    if ('string' === typeof options.executable) {
      this.executable = options.executable
      debug('use custom executable %s', options.executable)
    } else {
      this.executable = 'nfc-poll'
    }
  }

  this._output = ''

  EventEmitter.call(this)
}

Nfc.prototype.listen = function (callback) {
  if ('function' === typeof callback) {
    this.on('listenning', callback)
  }

  var self = this

  self._start()
  self.emit('listenning')

  process.once('exit', function () {
    self.close()
  })
}

Nfc.prototype.close = function (callback) {
  if ('function' === typeof callback) {
    this.on('close', callback)
  }

  this._shouldClose = true
  this._process.kill()
  this._clean()
}
