// Generated by BUCKLESCRIPT VERSION 2.1.0, PLEASE EDIT WITH CARE
'use strict';

var Nact = require("reason-nact/src/nact.js");

var system = Nact.start(/* None */0, /* () */0);

var ping = Nact.spawnStateless(/* Some */["ping"], /* None */0, system, (function (param, ctx) {
        console.log(param[1]);
        return Promise.resolve(Nact.Operators[/* <-< */0](param[0], /* Msg */[
                        ctx[/* self */2],
                        ctx[/* name */4]
                      ]));
      }));

var pong = Nact.spawnStateless(/* Some */["pong"], /* None */0, system, (function (param, ctx) {
        console.log(param[1]);
        return Promise.resolve(Nact.Operators[/* <-< */0](param[0], /* Msg */[
                        ctx[/* self */2],
                        ctx[/* name */4]
                      ]));
      }));

Nact.Operators[/* <-< */0](ping, /* Msg */[
      pong,
      "hello"
    ]);

exports.system = system;
exports.ping   = ping;
exports.pong   = pong;
/* system Not a pure module */
