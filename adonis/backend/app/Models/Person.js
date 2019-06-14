"use strict";

/** @type {typeof import('@adonisjs/lucid/src/Lucid/Model')} */
const Model = use("Model");

class Person extends Model {
  static get table() {
    return "persons";
  }

  measurements() {
    return this.hasMany("App/Models/Measurement");
  }
}

module.exports = Person;
