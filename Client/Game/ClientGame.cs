﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CardGameWPF.Network;
using CardGameWPF.Security;
using CardGameWPF.Enums;
using System.Timers;
using System.Windows.Controls;
using CardGameWPF.Data;
using System.Windows;

namespace CardGameWPF.Game
{
    public class ClientGame : IDisposable
    {
        private ClientNetwork network;
        private Timer timer;
        private MainWindow mainWindow;
        private ChatHandler chat;

        private void Update(Object source, ElapsedEventArgs e)
        {
            byte[] networkData = network.ReceiveData();
            if (networkData != null)
            {
                while (networkData.Any())
                {
                    var length = BitConverter.ToUInt16(networkData, 0);
                    networkData = networkData.Skip(sizeof(UInt16)).ToArray();
                    Packet packet = new Packet(networkData, length);
                    var packetHandler = PacketHandler.GetPacketHandler((SMSGPackets)packet.ReadUInt16());
                    packetHandler(packet, this);
                    packet.Dispose();

                    networkData = networkData.Skip(length).ToArray();
                }
            }

            timer.Start();
        }

        public ClientGame(string name, MainWindow window)
        {
            mainWindow = window;
            network = new ClientNetwork();
            chat = new ChatHandler(this);
            Player = new Player(this, MainWindow.PlayerCard1, MainWindow.PlayerCard2, MainWindow.PlayerCard3, MainWindow.PlayerCard4) { Name = name };
            Opponent = new Player(this, MainWindow.OpponentCard1, MainWindow.OpponentCard2, MainWindow.OpponentCard3, MainWindow.OpponentCard4);

            Packet packet = new Packet(CMSGPackets.CMSG_INIT_PACKET);
            RsaEncryptor.Inicialize();
            AesEncryptor.Inicialize();
            packet.Write(RsaEncryptor.Encrypt(AesEncryptor.Key));
            packet.Write(AesEncryptor.Encrypt(name));
            RsaEncryptor.Clear();

            SendPacket(packet);
            
            timer = new System.Timers.Timer(50);
            timer.AutoReset = false;
            timer.Elapsed += Update;
            timer.Start();
        }

        public MainWindow MainWindow { get { return mainWindow; } }
        public ChatHandler Chat { get { return chat; } }
        public Player Player { get; set; }
        public Player Opponent { get; set; }

        public Player GetPlayer(UInt32 playerId)
        {
            var player = Player ?? Opponent;
            if (player == null)
                return null;

            return (Player.Id == playerId) ? Player : Opponent;
        }

        public void SendPacket(Packet packet) 
        { 
            if (network != null)
                network.SendPacket(packet);
        }

        public void Dispose()
        {
            if (network != null)
            {
                timer.Stop();
                network.Dispose();
            }

            AesEncryptor.Clear();
        }

        public void SendChatMessage(string text, ChatTypes chatType, params string[] customParams)
        {
            if (chatType == ChatTypes.AutoSelect)
                chatType = chat.ActiveChat;

            chat.SendChatMessage(text, chatType, customParams);
        }

        public void HandleCommand(string command)
        {
            CommandHandler.HandleCommand(command, this);
        }

        public void Invoke(Action action)
        {
            mainWindow.Invoke(action);
        }

        public void SendSelectedCards()
        {
            Packet packet = new Packet(CMSGPackets.CMSG_SELECTED_CARDS);
            var cards = DataHolder.Cards.Where(x => x.Selected);

            packet.Write((byte)cards.Count());
            foreach (var card in cards)
            {
                packet.Write(card.Id);
                card.UnloadImages();
            }

            SendPacket(packet);
        }

        public void ShowCardDeck(bool showWaitGrid)
        {
            Invoke(new Action(delegate()
            {
                MainWindow.CardDeckGrid.Visibility = Visibility.Visible;
                MainWindow.WaitingGrid.Visibility = showWaitGrid ? Visibility.Visible : Visibility.Hidden;
            }));
        }

        // Unloads data
        public void UnloadData()
        {
            MainWindow.SlideShow.UnloadItems();
            DataHolder.UnloadData();
        }
    }
}