"use strict";

/** @type {import('@adonisjs/lucid/src/Schema')} */
const Schema = use("Schema");

class MeasurementDataSchema extends Schema {
  up() {
    this.create("measurement_data", table => {
      table.increments();

      table
        .integer("measurement_id")
        .unsigned()
        .references("id")
        .inTable("measurements");

      table.string("imu_serial");
      table.bigInteger("time_raw").notNullable();
      table.bigInteger("acc_x").notNullable();
      table.bigInteger("acc_y").notNullable();
      table.bigInteger("acc_z").notNullable();

      table.timestamps();
    });
  }

  down() {
    this.drop("measurement_data");
  }
}

module.exports = MeasurementDataSchema;
