DROP TABLE IF EXISTS SpellValues;
CREATE TABLE SpellValues
(
	Id INT NOT NULL PRIMARY KEY,
	SpellEffectId TINYINT NOT NULL,
	SpellAttributesMask INT NOT NULL,
	Target TINYINT NOT NULL,
	EffectValue1 TINYINT NOT NULL,
	EffectValue2 TINYINT NOT NULL,
	EffectValue3 TINYINT NOT NULL,
	EffectValue4 TINYINT NOT NULL
);

DROP TABLE IF EXISTS Spells;
CREATE TABLE Spells
(
	Id INT NOT NULL PRIMARY KEY,
	ManaCost TINYINT NOT NULL
);

DROP TABLE IF EXISTS SpellsSpellValues;
CREATE TABLE SpellsSpellValues
(
	SpellId INT NOT NULL,
	SpellValueId INT NOT NULL,
	FOREIGN KEY(SpellId) REFERENCES Spells(Id),
	FOREIGN KEY(SpellValueId) REFERENCES SpellValues(Id),
	PRIMARY KEY (SpellId, SpellValueId)
);
