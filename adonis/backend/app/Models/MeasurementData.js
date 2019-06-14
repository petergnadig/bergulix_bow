"use strict";

/** @type {typeof import('@adonisjs/lucid/src/Lucid/Model')} */
const Model = use("Model");

class MeasurementData extends Model {
  measurement() {
    return this.belongsTo("App/Models/Measurement");
  }
}

module.exports = MeasurementData;
