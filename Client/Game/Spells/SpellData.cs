﻿using System;

namespace Client.Game
{
    public class SpellData
    {
        public UInt32 SpellId { get; private set; }
        public string Name { get; private set; }
        public string Description { get; private set; }

        public SpellData(UInt32 spellId, string name, string description)
        {
            SpellId = spellId;
            Name = name;
            Description = description;
        }
    }
}
