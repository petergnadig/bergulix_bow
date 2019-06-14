"use strict";

/** @type {typeof import('@adonisjs/lucid/src/Lucid/Model')} */
const Model = use("Model");

class Measurement extends Model {
  person() {
    return this.belongsTo("App/Models/Person");
  }

  bow() {
    return this.belongsTo("App/Models/Bow");
  }

  data() {
    return this.hasMany("App/Models/MeasurementData");
  }
}

module.exports = Measurement;
