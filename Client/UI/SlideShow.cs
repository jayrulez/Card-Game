﻿using Client.Data;
using Client.Enums;
using Client.Game;
using System;
using System.Linq;
using System.Timers;
using System.Windows;
using System.Windows.Media.Animation;

namespace Client.UI
{
    public class SlideShow
    {
        private static readonly int sliderSize = 10;
        private int index;
        private MainWindow mainWindow;
        private Timer timer;

        public Card SelectedCard { get; private set; }

        public SlideShow(MainWindow window)
        {
            timer = new Timer()
            {
                AutoReset = false,
            };
            timer.Elapsed += delegate(object sender, ElapsedEventArgs e) { LoadItems(); };
            index = 0;
            mainWindow = window;
        }

        public void NextElements()
        {
            if (((index + 1) * sliderSize) < DataHolder.Cards.Count())
            {
                index++;
                RefreshImageList("FadeLstBox");
                OnClickNavigationButton();
            }
        }

        public void PreviousElements()
        {
            if ((index * sliderSize) > 0)
            {
                index--;
                RefreshImageList("FadeLstBoxBack");
                OnClickNavigationButton();
            }
        }

        private void OnClickNavigationButton()
        {
            timer.Stop();
            timer.Interval = 400;
            timer.Start();
        }

        public void LoadItems()
        {
            mainWindow.Invoke(new Action(delegate()
            {
                mainWindow.Images.DataContext = DataHolder.Cards.Skip(index * sliderSize).Take(sliderSize);
                mainWindow.Images.SelectedIndex = -1;
            }));
        }

        public void UnloadItems()
        {
            mainWindow.Invoke(new Action(delegate()
            {
                mainWindow.Images.DataContext = null;
                mainWindow.Images.SelectedIndex = -1;
                mainWindow.imgMain = null;
            }));
        }

        public void SetVisible(bool visible)
        {
            mainWindow.Invoke(new Action(delegate()
            {
                mainWindow.SlideShowGrid.Visibility = visible ? Visibility.Visible : Visibility.Hidden;
            }));
        }

        public void ItemSelected(Card card)
        {
            if (card == null)
                return;

            mainWindow.CardName.Text = $"Name: {card.Name}";
            mainWindow.CardDamage.Text = $"Damage: {card.Damage}";
            mainWindow.CardHitPoints.Text = $"Hit points: {card.Hp}";
            mainWindow.CardMana.Text = $"Mana: {card.Mana}";
            mainWindow.CardDefense.Text = $"Defense: {card.Defense}";
            mainWindow.imgMain.Source = card.Image;

            mainWindow.SetSelectCard(card.SelectionType == SelectionType.Selected);
            SelectedCard = card;
        }

        private void RefreshImageList(string storyboardName)
        {
            if (mainWindow.FindResource(storyboardName) is Storyboard storyboard)
                storyboard.Begin();
        }
    }
}
