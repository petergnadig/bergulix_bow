"use strict";

/** @type {import('@adonisjs/lucid/src/Schema')} */
const Schema = use("Schema");

class MeasurementSchema extends Schema {
  up() {
    this.create("measurements", table => {
      table.increments();

      table
        .integer("person_id")
        .unsigned()
        .references("id")
        .inTable("persons");

      table
        .integer("bow_id")
        .unsigned()
        .references("id")
        .inTable("bows");

      table.timestamps();
    });
  }

  down() {
    this.drop("measurements");
  }
}

module.exports = MeasurementSchema;
